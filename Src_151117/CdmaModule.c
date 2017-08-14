
/******************************************************************************
*
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved
*
* �� �� ���� CdmaModule.C
* ��    ���� CDMA���ݽ���ģ��

* �������ڣ� 2015��4��24��10:15:40
* ��    �ߣ� Bob
* ��ǰ�汾�� V1.00

------------------------------------------------------
*******************************************************************************/
#define  CDMA_EXT
#include "msp430x54x.h"
#include "global.h"
#include "CdmaModule.h"
#include "DataType.h"
#include "Uart0.h"
#include "Uart3.h"
#include "SystemCtl.h"
#include "string.h"
#include "Timer.h"
#include "SoftTimer.h"
#include "GpsParse.h"
#include "stdio.h"
#include "OS_MEM.h"
#include "Rtc.h"
#include "BeiDouModule.h"

//ǰ���ǿ������ת��ȱʡ����ɼ������
#define _GetWordFromAddr(pAddr)         (UINT16)(((UINT16)pAddr[0]) << 8) + pAddr[1]

#define _GetDoubleWordFromAddr(pAddr)   ((UINT32)(((UINT32)pAddr[0]) << 24) +    \
                                         (UINT32)(((UINT32)pAddr[1]) << 16) +    \
                                         (UINT32)(((UINT32)pAddr[2]) << 8) + pAddr[3])

//������ĸ��ӣ�ȡ��8λ��Ϊָ�����
#define CMD_ZCANS         0x7F
#define CMD_ZCCNT         0x82
#define CMD_CMTI          0x2D
#define CMD_ZCEND         0x74
#define CMD_CSQ           0xE7
#define CMD_SMS           0x68
#define CMD_CMGR          0x29
#define CMD_NONE          0x00

#define CMD_ASSERT(a)    ((a == CMD_CMTI ) || \
                          (a == CMD_CMGR ) || \
                          (a == CMD_CSQ  ))


//CDMA����״̬
typedef enum
{
    //�������ͷ
    E_CDMA_RECEIVING_HEADER,
    //���յ�������
    E_CDMA_RECEIVING_CMD,
    //��������
    E_CDMA_RECEIVING_BODY,
    //SMS READY����
    E_CDMA_RECEIVING_SMS,
    //��Ч��־״̬
    E_CDMA_RECEIVING_VALID

}teCdmaAssembleState;

/* ��������ṹ���� */
typedef struct
{
  UINT8 *pu8AssembleBuf;  //���������ָ��

  UINT8 u8Cmd;            //cdma��������

  UINT16 u16BufDataLen ;  //��������ݳ���

  UINT8 u8MsgEnvent;      //�յ����ź�ģ�������������һϵ��ָ�
                          //4��ָ��ֱ��Ӧ��4λ

  teCdmaAssembleState eAssembleState;  //���״̬

  SINT8 sCdmaDog;         //cdma�������ʱ��ʱ��

  SINT8 sCsqDog;          //CSQ�ź�ǿ�Ȼ�ȡʱ�䶨ʱ��

}_tsCdmaCmdCb;

_tsCdmaCmdCb sCdmaCmdCb;

//����δ������ʱ������Ӧ��λ�ñ�ʶΪ1������Ϊ0
UINT8 au8MsgUnRead[MSG_MAX_SIZE];
BOOL  bMsgReady = FALSE;

//���嵱ǰ���յ��Ķ��ű��
SINT8 s8CurrentIndex = -1;

//����DTU����ģʽ
const UINT8 u8ConfigMode[]         = {"enter_config_mode\0"};
const UINT8 u8ConfigStart[]        = {0xff,0xff,0xff,0xff};
const UINT8 u8ConfigRxData[]       = {"aaaaaaaaaaaaaaaa\0"};

const UINT8 au8BackOk[3]           = {0x4f, 0x4b, 0x00};

//��ѯģ���ͺ�
const UINT8 au8GetModuleIndex[]    = {"AT+CGMM\r"};
//���ڲ���������
const UINT8 au8UartSet[]           = {"AT+UART=9600,ON,8,N,1\r"};
//����32khz���˯��ģʽ
const UINT8 au8EnableZDSLEEP[]     = {"AT+ZDSLEEP=1\r"};
const UINT8 au8DisableZDSLEEP[]    = {"AT+ZDSLEEP=0\r"};
//DTR�������߿���
const UINT8 au8EnableZDSLEEPDTR[]  = {"AT+ZDSLEEPDTR=1\r"};
const UINT8 au8DisableZDSLEEPDTR[] = {"AT+ZDSLEEPDTR=0\r"};
//��ȡCDMAʱ��
const UINT8 au8GetCdmaTime[]       = {"AT+ZDSLEEPDTR=2\r"};
//��λģ��
const UINT8 au8DisableZRST[]       = {"AT+ZRST=0\r"};
//����λ,1���Ӻ�λ
//const UINT8 au8EnableZRST[] = {"AT+ZRST=1,"00:01"\r"};

//�ź�ǿ�Ȳ�ѯ��
//���أ�+CSQ: 28,99 OK��16���¿��ܺ���ʧ�ܡ�
const UINT8 au8CSQ[]               = {"AT+CSQ\r"};

//ʱ�Ӳ�ѯ
//���أ�+CCLK:"2004/02/09��17��34��23.694"��ǰ����ʱ�䡣
const UINT8 au8CCLK[]              = {"AT+CCLK?\r"};

//���ö���Ϣģʽ
const UINT8 au8CMGF[]              = {"AT+CMGF=1\r"};
//��ȡ����Ϣ
const UINT8 au8CMGR[]              = {"AT+CMGR="};

//���ñ�������
//const UINT8 au8SetCNUM[] = {"AT+CNUM="};
//const UINT8 au8GetCNUM[] = {"AT+CNUM?"};

//ģ��ָʾ
const UINT8 au8ZIND[] = {"AT+ZIND="};

/////////////////////////////////////////////////////////////
// ���ŷ���
//ѡ����ŷ���
const UINT8 au8MsgType[]            = {"AT+CSMS?\r"};
//���ö���ΪӢ�ģ� ascii ��ʽ
const UINT8 au8ZMSGL_ASCII[]        = {"AT+ZMSGL=1,2\r"};
//���ö���Ϊ���� �� unicode��ʽ
const UINT8 au8ZMSCL_UNICODE[]      = {"AT+ZMSGL=6,4\r"};

//����ɾ��
const UINT8 au8CMGD[] = {"AT+CMGD="};
//���Ͷ�Ϣ
typedef struct _tsCMGS
{
  UINT8 *pCmd; //���AT���� AT+CMGS=

  UINT8 *pPhoneNum;  //��ŵ绰���룬��ǰ��ġ���

  UINT8 * au8Info;    //�����������Ϣָ��

}tsCMGS, *pCMGS;

//��������
void CdmaCmdInit(void);


/*****************************************************************************
 �� �� ��  : CdmaSendMsg
 ��������  : CDMAģ�鷢�Ͷ���

 �������  : none
 �� �� ֵ  : none,
*****************************************************************************/
BOOL CdmaSendMsg(const UINT8 * ptr, UINT8 u8Len)
{
//  Uart0EmptyRxBuf();
  Uart0SendBuffer(ptr, u8Len);
  return 1;
}

/*****************************************************************************
 �� �� ��  : AssembleCdmaMsg
 ��������  : AssembleCdmaMsg

 �������  : *pNum -- ���ͺ����ָ��
             *pBuf -- ������Ϣָ��

 �� �� ֵ  : TRUE, FALSE
*****************************************************************************/
const UINT8 au8CMGS[8] = {"AT+CMGS="};
BOOL AssembleCdmaMsg(UINT8 *pNum, UINT8 *pBuf, UINT8 u8Len)
{
  UINT8 *pMsg;
//  UINT8 *pRxBuf;
  UINT8 i=0;
//  SINT8 s8Dog;
//  UINT8 u8Back;

  pMsg = OSMemGet(pSmallMem, &err);
//  pRxBuf = OSMemGet(pSmallMem, &err);

  memcpy(&pMsg[i], &au8CMGS[0], 8);
  i += 8;
  pMsg[i++] = '"';
  memcpy(&pMsg[i], pNum, 11);
  i += 11;
  pMsg[i++] = '"';
  pMsg[i++] = '\r';

  memcpy(&pMsg[i], pBuf, u8Len);
  i += u8Len;

  pMsg[i++] = 0x1A;   //������

  CdmaSendMsg(pMsg, i);
  g_u8TimerDelay=0;
  while(g_u8TimerDelay < 10);    //��ʱ20ms

  OSMemPut(pSmallMem, pMsg);

  return TRUE;
 /*
  StopWatchDog();
  s8Dog = SwdGet();
  SwdSetLifeTime(s8Dog, 2000);
  SwdEnable(s8Dog);
  CdmaSendMsg(pMsg, i);

  while(TRUE)
  {
    if(Uart0GetRxDataLength() > 21)
    {
      Uart0GetRxData(&pRxBuf[0], 22);
      i=0;
      while((pRxBuf[i] != 'O') && (i<22))
      {
        i += 1;
      }
      OSMemPut(pSmallMem, pMsg);
      OSMemPut(pSmallMem, pRxBuf);
      SwdFree(s8Dog);

      return TRUE;
    }

    if(SwdGetLifeTime(s8Dog) == 0)
    {
      SwdReset(s8Dog);

      //����10�����ݻ�û�н��յ����ֱ�ӷ���
      ++u8Back;
      if(u8Back > 10)
      {
        OSMemPut(pSmallMem, pMsg);
        OSMemPut(pSmallMem, pRxBuf);
        SwdFree(s8Dog);

        return FALSE;
      }

      //��մ��ڻ�����
//      Uart0EmptyRxBuf();
      //����ģ���ͺ�����
      CdmaSendMsg(pMsg, i);
     }
  }*/
}

void CdmaToBeiDou(void)
{
  g_bCdmaSend = FALSE;
  g_bBdSend		= TRUE;
  CdmaPowerOff();

  BeiDouPowerOn();
  BdSendCmdICJC();
}

void BeiDouToCdma(void)
{
  g_bCdmaSend = TRUE;
  g_bBdSend	  = FALSE;

  CdmaPowerOn();
  StopWatchDog();           // �رտ��Ź�
  CdmaInit();
  FreeWatchDog();          //�ͷſ��Ź�

  BeiDouPowerOff();
}


/*****************************************************************************
 �� �� ��  : DeletCdmaMsg()
 ��������  : ����ɾ������

 �������  : u8Index -- ��Ҫɾ���Ķ��ű��, ���洢30������
                        0xff -- ɾ��ȫ���Ķ���Ϣ

 �� �� ֵ  : TRUE, FALSE
*****************************************************************************/
BOOL DeletCdmaMsg(SINT8 s8Index)
{
    UINT8   au8Buf[24];
    UINT8   i= 0;
    UINT8 * pMsg;
//    SINT8   s8Dog;
//    UINT8   u8Back = 0;
//   UINT8   au8RxBuf[16];

    if((s8Index > MSG_MAX_SIZE) || (s8Index < 0))
    {
        return FALSE;
    }

    memset(&au8Buf[0], 0, 24);
    pMsg = &au8Buf[0];

    memcpy(&pMsg[i], &au8CMGD[0], 8);
    i += 8;

    if(s8Index == MSG_MAX_SIZE)
    {
        pMsg[i++] = 0x31;
        pMsg[i++] = ',';
        pMsg[i++] = 0x34;
    }
    else
    {
        if(s8Index < 9)
        {
            pMsg[i++] = s8Index + '0';
        }
        else
        {
            pMsg[i++] = s8Index / 10 + '0';
            pMsg[i++] = s8Index % 10 + '0';
        }
    }
    pMsg[i++] = '\r';

    //����ģ��ɾ������
    CdmaSendMsg(pMsg, i);
    
    return TRUE;

    //��ͬʱ���մ�����Ϣʱ���ȴ�ʱ���������ֱ�Ӻ��Ե�������Ϣ��
/*
    StopWatchDog();
    s8Dog = SwdGet();
    SwdSetLifeTime(s8Dog, 5000);
    SwdEnable(s8Dog);
    memset(&au8RxBuf[0], 0, 16);

    while(TRUE)
    {
        if(Uart0GetRxDataLength() > 2)
        {
          Uart0GetRxData(&au8RxBuf[0], 2);

          if(0 == memcmp(&au8RxBuf[0], "OK", 2))
          {
            return TRUE;
          }
        }

        if(SwdGetLifeTime(s8Dog) == 0)
        {
          SwdReset(s8Dog);

          //����5�����ݻ�û�н��յ����ֱ�ӷ���
          ++u8Back;
          if(u8Back > 2)
          {
            SwdFree(s8Dog);

            return FALSE;
          }

           //����ģ��ɾ������
           CdmaSendMsg(pMsg, i);
        }
    }*/
}

/*****************************************************************************
 �� �� ��  : CdmaInit
 ��������  : CDMAģ���ʼ��

 �������  : none
 �� �� ֵ  : none,
*****************************************************************************/
void CdmaInit(void)
{
  CdmaResetDir |= CdmaReset;
  CdmaOnOffDir |= CdmaOnOff;

  //IC�����
//  CdmaIcCheckSetIn();
  //CDMA RI�жϣ��½��ش����ж�
  CdmaRiIesEdge();
  CdmaRiDir &= ~CdmaRi;

  //cdmaģ�鸴λ
  SetCdmaReset(0);
  g_u16TimerDelay=0;
  while(g_u16TimerDelay < 2);    //�͵�ƽ��ʱ20ms
  SetCdmaReset(1);
  //cdmaģ�鿪��
  SetCdmaOnOff(1);
  g_u16TimerDelay=0;
  while(g_u16TimerDelay < 1);    //��ʱ10ms
  SetCdmaOnOff(0);
  g_u16TimerDelay=0;
  while(g_u16TimerDelay < 100);  //��ʱ2s
  SetCdmaOnOff(1);
  g_u16TimerDelay=0;
  while(g_u16TimerDelay < 100);  //��ʱ5s��ʱ��̫�̻��ⲻ���ֻ���

  //���һ���Ƿ����IC��
#if DTIC_DET_EN > 0
  if(!CdmaIcDet())
  {
    if(g_bDebug & CDMA_DEBUG)
    {
      Uart3SendString("#CDMA IC Card Not Found!\r\n ");
    }

    CdmaToBeiDou();
//    return;
  }
  else
#endif
  {
    /*
    if(GetCdmaCSQ() > 20)					//�ź�ǿ�ȿ��Է�����Ϣ
    {
      g_bCdmaSend = TRUE;
      g_bBdSend   = FALSE;

      BeiDouPowerOff();

      CdmaSendMsg(au8ZMSGL_ASCII, sizeof(au8ZMSGL_ASCII));

      CdmaDtrSetLow();             //����DTR����CDMA�˳�������
      CdmaDtrSetOut();
      CdmaLowPowerEnter();

      CdmaIndicatorEnable();
    }
    else
    {
      CdmaToBeiDou();
    }   */

    g_bCdmaSend = TRUE;
    g_bBdSend   = FALSE;

    CdmaSendMsg(au8ZMSGL_ASCII, sizeof(au8ZMSGL_ASCII));
    CdmaDtrSetLow();             //����DTR����CDMA�˳�������
    CdmaDtrSetOut();
    CdmaLowPowerEnter();
 //   CdmaIndicatorEnable();

    CdmaSendMsg(&au8CMGF[0], sizeof(au8CMGF));

    //cdmaģ��������������ʼ����
    CdmaCmdInit();

    //������ջ���������
    Uart0EmptyRxBuf();
  }
}

/*****************************************************************************
 �� �� ��  : SetLocationNbr
 ��������  : ���ñ�������

 �������  : PTR
 �� �� ֵ  : TRUE , FALSE
*****************************************************************************/
BOOL SetLocationNbr(UINT8 *ptr)
{
  UINT8 au8SetCNUM[24];
  UINT8 i,u8Back;
  SINT8 sDog;

  i = 0;
  memcpy(&au8SetCNUM[i], "AT+CNUM=", 8);
  i += 8;

  memcpy(&au8SetCNUM[i], ptr, 11);
  i += 11;

  au8SetCNUM[i++] = '\r';

  //��մ��ڻ�����
//  Uart0EmptyRxBuf();
  //����ģ���ͺ�����
  CdmaSendMsg(au8SetCNUM, i);

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);
  StopWatchDog();

  while(TRUE)
  {
    if(Uart0GetRxDataLength() > 1)
    {
      if(Uart0GetByte() == 'O')
      {
        if(Uart0GetByte() == 'K')
        {
          SwdFree(sDog);
          return TRUE;
        }
      }
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //����5�����ݻ�û�н��յ����ֱ�ӷ���
      ++u8Back;
      if(u8Back > 5)
      {
        SwdFree(sDog);
        return FALSE;
      }

      //��մ��ڻ�����
      Uart0EmptyRxBuf();
      //����ģ���ͺ�����
      CdmaSendMsg(au8SetCNUM, i);
     }
  }

}

/*****************************************************************************
 �� �� ��  : GetLocationNbr
 ��������  : ���ñ�������

 �������  : NONE
 �� �� ֵ  : TRUE , FALSE
*****************************************************************************/
BOOL GetLocationNbr(UINT8 *ptr)
{
  UINT8 au8GetCNUM[16];
  UINT8 au8Buf[48];
  UINT8 i,u8Back;
  SINT8 sDog;

  memcpy(&au8GetCNUM[0], "AT+CNUM?\r", 10);

  //��մ��ڻ�����
  Uart0EmptyRxBuf();
  //����ģ���ͺ�����
  CdmaSendMsg(au8GetCNUM, 10);

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);
  StopWatchDog();

  while(TRUE)
  {
    if(Uart0GetRxDataLength() > 34)
    {
      Uart0GetRxData(au8Buf, 34);

      i = 0;
      while((au8Buf[i] != ':') && (i<34))
      {
        i++;
      }

      i += 1;
      memcpy(ptr, &au8Buf[i], 11);

      SwdFree(sDog);
      return TRUE;
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //����5�����ݻ�û�н��յ����ֱ�ӷ���
      ++u8Back;
      if(u8Back > 5)
      {
        SwdFree(sDog);
        return FALSE;
      }

      //��մ��ڻ�����
      Uart0EmptyRxBuf();
      //����ģ���ͺ�����
      CdmaSendMsg(au8GetCNUM, i);
     }
  }
}

/*****************************************************************************
 �� �� ��  : GetCdmaModule
 ��������  : ��ȡCDMAģ���ͺ�

 �������  : none
 �� �� ֵ  : none,
*****************************************************************************/
void GetCdmaModule(void)
{
  UINT8 i,u8Back;
  SINT8 sDog;
  UINT8 *pRxBuf;

  pRxBuf = OSMemGet(pSmallMem, &err);
//  memset(pRxBuf, 0, BUF_SMALL_SIZE);

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);
  StopWatchDog();

  //��մ��ڻ�����
  Uart0EmptyRxBuf();
  //����ģ���ͺ�����
  CdmaSendMsg(au8GetModuleIndex, sizeof(au8GetModuleIndex));

  while(TRUE)
  {
    if(Uart0GetRxDataLength() > 19)
    {
      Uart0GetRxData(pRxBuf, 20);

      i = 0;
      while((pRxBuf[i] != '\r') && (i<20))
      {
        i++;
      }

      if(memcmp(&pRxBuf[i-6], "MC8332", 6) == 0)
      {
        Uart3SendString("#Detect ZTE CDMA Module. The Product Model Is MC8332!\r\n");
      }
      else
      {
        Uart3SendString("#CDMA Module To Find The Failure!\r\n");
      }

      break;
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);
      i = 0;

      //����5�����ݻ�û�н��յ����ֱ�ӷ���
      ++u8Back;
      if(u8Back > 5)
        break;

      //��մ��ڻ�����
      Uart0EmptyRxBuf();
      //����ģ���ͺ�����
      CdmaSendMsg(au8GetModuleIndex, sizeof(au8GetModuleIndex));
    }
  }

  //�ͷ��ڴ�
  OSMemPut(pSmallMem, pRxBuf);
  //�ͷ������ʱ��
  SwdFree(sDog);
}

/*****************************************************************************
 �� �� ��  : GetCdmaCSQ
 ��������  : ��ȡCDMA�ź�ǿ��

 �������  : none
 �� �� ֵ  : �ź�ǿ��,0~31
*****************************************************************************/
UINT8 GetCdmaCSQ(void)
{
  UINT8 i,u8Back;
  SINT8 sDog;
  UINT8 *pRxBuf;
  UINT8 u8Temp;

  pRxBuf = OSMemGet(pSmallMem, &err);
//  memset(pRxBuf, 0, BUF_SMALL_SIZE);

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);
  //��մ��ڻ�����
  Uart0EmptyRxBuf();
  //����ģ���ͺ�����
  CdmaSendMsg(au8CSQ, sizeof(au8CSQ));
  StopWatchDog();

  while(TRUE)
  {
    if(Uart0GetRxDataLength() > 24)
    {
      Uart0GetRxData(pRxBuf, 24);

      i = 0;
      while((pRxBuf[i] != ':') && (i<24))
      {
        i++;
      }

      if((pRxBuf[i+3] >= 0x30) && (pRxBuf[i+2] >= 0x30))
      {
        u8Temp = (pRxBuf[i+2]-0x30)*10 + (pRxBuf[i+3]-0x30);
        break;
      }
     }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //����10�����ݻ�û�н��յ����ֱ�ӷ���
      ++u8Back;
      if(u8Back > 5)
      {
        CdmaToBeiDou();
        u8Temp = 0xFF;
        break;
      }

      //��մ��ڻ�����
      Uart0EmptyRxBuf();
      //����ģ���ͺ�����
      CdmaSendMsg(au8CSQ, sizeof(au8CSQ));
    }
  }

  //�ͷ��ڴ�
  OSMemPut(pSmallMem, pRxBuf);
  //�ͷ������ʱ��
  SwdFree(sDog);

  return u8Temp;
}

/*****************************************************************************
 �� �� ��  : GetCdmaTime
 ��������  : ��ȡCDMAʱ��, ���õ�RTC

 �������  : *pTime��*pu8Len
 �� �� ֵ  : none
*****************************************************************************/
void GetCdmaTime(UINT8 *pTime, UINT8 *pu8Len)
{
  UINT8 i,j,u8Back;
  SINT8 sDog;
  UINT8 *pRxBuf;

  pRxBuf = OSMemGet(pSmallMem, &err);
//  memset(pRxBuf, 0, BUF_SMALL_SIZE);

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);
  //��մ��ڻ�����
  Uart0EmptyRxBuf();
  //����ģ���ͺ�����
  CdmaSendMsg(au8CCLK, sizeof(au8CCLK));

  while(TRUE)
  {
    if(Uart0GetRxDataLength() > 48)
    {
      Uart0GetRxData(pRxBuf, 48);

      i = 0;
      while((pRxBuf[i] != '"') && (i<48))
      {
        i++;
      }

      j = ++i;
      while((pRxBuf[j] != '"') && (j<48))
      {
        j++;
      }

      break;
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //����10�����ݻ�û�н��յ����ֱ�ӷ���
      ++u8Back;
      if(u8Back > 5)
        break;

      //��մ��ڻ�����
      Uart0EmptyRxBuf();
      //����ģ���ͺ�����
      CdmaSendMsg(au8CCLK, sizeof(au8CCLK));
    }
  }
  //ʱ�俽��������ms����
  *pu8Len = j-i;
  memcpy(pTime, &pRxBuf[i], j-i);
  //�ͷ��ڴ�
  OSMemPut(pSmallMem, pRxBuf);
  //�ͷ������ʱ��
  SwdFree(sDog);
}


/*****************************************************************************
 �� �� ��  : SetCdmaTimeToRtc
 ��������  : ��ȡCDMAʱ��, ���õ�RTC

 �������  : none
 �� �� ֵ  : TRUE -- У׼�ɹ��� FALSE -- У׼ʧ��
*****************************************************************************/
BOOL SetCdmaTimeToRtc(void)
{
  UINT8 au8Time[32];
  UINT8 u8Len;
  UINT8 i;
  _tsTimeCb sTime;

  GetCdmaTime(&au8Time[0], &u8Len);

  i = 2;   //���ƫ��
  if((au8Time[i] >= 0x30) && (au8Time[i] <= 0x39))
    sTime.Year = (au8Time[i] - 0x30)*10 + (au8Time[i+1] - 0x30);
  else
    return FALSE;

  i += 3;   //�·�ƫ��
  if((au8Time[i] >= 0x30) && (au8Time[i] <= 0x39))
    sTime.Month = (au8Time[i] - 0x30)*10 + (au8Time[i+1] - 0x30);
  else
    return FALSE;

  i += 3;   //����ƫ��
  if((au8Time[i] >= 0x30) && (au8Time[i] <= 0x39))
    sTime.Date = (au8Time[i] - 0x30)*10 + (au8Time[i+1] - 0x30);
  else
    return FALSE;

  i += 3;   //ʱƫ��
  if((au8Time[i] >= 0x30) && (au8Time[i] <= 0x39))
    sTime.Hour = (au8Time[i] - 0x30)*10 + (au8Time[i+1] - 0x30);
  else
    return FALSE;

  i += 3;   //��ƫ��
  if((au8Time[i] >= 0x30) && (au8Time[i] <= 0x39))
    sTime.Minute = (au8Time[i] - 0x30)*10 + (au8Time[i+1] - 0x30);
  else
    return FALSE;

  i += 3;   //��ƫ��
  if((au8Time[i] >= 0x30) && (au8Time[i] <= 0x39))
    sTime.Second = (au8Time[i] - 0x30)*10 + (au8Time[i+1] - 0x30);
  else
    return FALSE;

  RtcConverHour(1);
//  SetRtcInt1Out();
  SetTimeToRtc(&sTime);

  //��RTC�ⲿ�ж�
  SetRtcInt1Out();

  return TRUE;
}

/*****************************************************************************
 �� �� ��  : CdmaLowPowerEnter
 ��������  : CDMAģ�����͹���ģ��

 �������  : PTR
 �� �� ֵ  : TRUE , FALSE
*****************************************************************************/
void CdmaLowPowerEnter(void)
{
  UINT8 au8RxBuf[24];
  UINT8 i,u8Back=0;
  SINT8 sDog;

  memset(au8RxBuf, 0, 24);

  //����CDMA 32k���˯��
  CdmaSendMsg(&au8EnableZDSLEEP[0], sizeof(au8EnableZDSLEEP));
  g_u8TimerDelay=0;
  while(g_u8TimerDelay < 10);  //��ʱ100ms���ȴ��ظ�����

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);

  while(TRUE)
  {
    if(Uart0GetRxDataLength() > 17)
    {
      Uart0GetRxData(au8RxBuf, 18);

      i = 0;
      while((au8RxBuf[i] != 'O') && (i<18))
      {
        i++;
      }

      break;
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //����10�����ݻ�û�н��յ����ֱ�ӷ���
      ++u8Back;
      if(u8Back > 5)
      {
        CdmaToBeiDou();
        //�ͷ������ʱ��
        SwdFree(sDog);
        return;
      }

      //����ģ���ͺ�����
      CdmaSendMsg(&au8EnableZDSLEEP[0], sizeof(au8EnableZDSLEEP));
    }
  }

  memset(au8RxBuf, 0, 24);

  //����DTR�ܽſ��ƽ������
  CdmaSendMsg(&au8EnableZDSLEEPDTR[0], sizeof(au8EnableZDSLEEPDTR));
  while(g_u8TimerDelay < 10);  //��ʱ100ms���ȴ��ظ�����

   while(TRUE)
  {
    if(Uart0GetRxDataLength() > 19)
    {
      Uart0GetRxData(au8RxBuf, 20);

      i = 0;
      while((au8RxBuf[i] != 'O') && (i<20))
      {
        i++;
      }

      break;
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //����10�����ݻ�û�н��յ����ֱ�ӷ���
      ++u8Back;
      if(u8Back > 5)
      {
        CdmaToBeiDou();
        //�ͷ������ʱ��
        SwdFree(sDog);
        return;
      }

      //����ģ���ͺ�����
      CdmaSendMsg(&au8EnableZDSLEEPDTR[0], sizeof(au8EnableZDSLEEPDTR));
    }
  }
  SwdFree(sDog);
}

/*****************************************************************************
 �� �� ��  : CdmaLowPowerExit
 ��������  : CDMAģ���˳��͹���ģ��

 �������  : PTR
 �� �� ֵ  : TRUE , FALSE
*****************************************************************************/
void CdmaLowPowerExit(void)
{
  UINT8 au8RxBuf[24];
  UINT8 i,u8Back=0;
  SINT8 sDog;

  memset(au8RxBuf, 0, 24);

  //��ֹCDMA 32k���˯��
  CdmaSendMsg(&au8DisableZDSLEEP[0], sizeof(au8DisableZDSLEEP));
  g_u8TimerDelay=0;
  while(g_u8TimerDelay < 10);  //��ʱ100ms���ȴ��ظ�����

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);

  while(TRUE)
  {
    if(Uart0GetRxDataLength() > 17)
    {
      Uart0GetRxData(au8RxBuf, 18);

      i = 0;
      while((au8RxBuf[i] != 'O') && (i<18))
      {
        i++;
      }

      break;
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //����10�����ݻ�û�н��յ����ֱ�ӷ���
      ++u8Back;
      if(u8Back > 5)
      {
        CdmaToBeiDou();
        //�ͷ������ʱ��
        SwdFree(sDog);
        return;
      }

      //����ģ���ͺ�����
      CdmaSendMsg(&au8DisableZDSLEEP[0], sizeof(au8DisableZDSLEEP));
    }
  }

  memset(au8RxBuf, 0, 24);

  //��ֹDTR�ܽſ��ƽ������
  CdmaSendMsg(&au8DisableZDSLEEPDTR[0], sizeof(au8DisableZDSLEEPDTR));
  while(g_u8TimerDelay < 10);  //��ʱ100ms���ȴ��ظ�����

   while(TRUE)
  {
    if(Uart0GetRxDataLength() > 19)
    {
      Uart0GetRxData(au8RxBuf, 20);

      i = 0;
      while((au8RxBuf[i] != 'O') && (i<20))
      {
        i++;
      }

      break;
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //����10�����ݻ�û�н��յ����ֱ�ӷ���
      ++u8Back;
      if(u8Back > 5)
      {
        CdmaToBeiDou();
        //�ͷ������ʱ��
        SwdFree(sDog);
        return;
      }

      //����ģ���ͺ�����
      CdmaSendMsg(&au8DisableZDSLEEPDTR[0], sizeof(au8DisableZDSLEEPDTR));
    }
  }

  SwdFree(sDog);
}

/*****************************************************************************
 �� �� ��  : CdmaWakeUpCfg
 ��������  : CDMAģ���˳�˯��ģʽ������ȡCDMA�źţ��ź�ǿ�ȵ���16ʱ�Զ��л���
             ����ͨѶģ����

 �������  : PTR
 �� �� ֵ  : TRUE , FALSE
*****************************************************************************/
void CdmaWakeUpCfg(void)
{
  CdmaDtrSetLow();     //����DTR����CDMA���ѡ�();

  g_u16TimerDelay=0;
  while(g_u16TimerDelay < 200);  //��ʱһ�£��ȴ�ģ���ȶ�

  if(GetCdmaCSQ() > 16)					//�ź�ǿ�ȿ��Է�����Ϣ
  {
    g_bCdmaSend = TRUE;
    g_bBdSend   = FALSE;

     BeiDouPowerOff();
  }
  else
  {
    CdmaToBeiDou();
  }
}

/*****************************************************************************
 �� �� ��  : CdmaCmdInit
 ��������  : CDMAģ��������ճ�ʼ��

 �������  : none
 �� �� ֵ  : none,
*****************************************************************************/
void CdmaCmdInit(void)
{
  sCdmaCmdCb.u16BufDataLen  = 0;
  sCdmaCmdCb.u8MsgEnvent    = 0;
  sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_HEADER;
  sCdmaCmdCb.pu8AssembleBuf = OSMemGet(pSmallMem, &err);

  //Get One soft timer
  sCdmaCmdCb.sCdmaDog       = SwdGet();
  SwdSetName(sCdmaCmdCb.sCdmaDog, "CdmaTimer");
  SwdSetLifeTime(sCdmaCmdCb.sCdmaDog, 500);

  sCdmaCmdCb.sCsqDog       = SwdGet();
  SwdSetName(sCdmaCmdCb.sCsqDog, "CSQTimer");
  SwdSetLifeTime(sCdmaCmdCb.sCsqDog, 40000);     //ÿ45���ȡһ���ź�ǿ��
  SwdEnable(sCdmaCmdCb.sCsqDog);

  memset(&au8MsgUnRead[0], 0, MSG_MAX_SIZE);
  bMsgReady = FALSE;
}

/*****************************************************************************
 �� �� ��  : CdmaCmdAssemble
 ��������  : CDMA�����������

 �������  : ��
 �� �� ֵ  : �� , ��
*****************************************************************************/
void CdmaCmdAssemble(UINT8 u8InData)
{
    switch(sCdmaCmdCb.eAssembleState)
    {
        case E_CDMA_RECEIVING_HEADER:
        {
            if(u8InData == '+')
            {
               sCdmaCmdCb.pu8AssembleBuf[0] = u8InData;
               sCdmaCmdCb.u8Cmd             = 0;
               sCdmaCmdCb.u16BufDataLen     = 1;
               sCdmaCmdCb.eAssembleState    = E_CDMA_RECEIVING_CMD;
            }
/*            else
            {
                if((u8InData == 'S')  && (sCdmaCmdCb.u8MsgEnvent & 0x0C == 0x0C))
                {
                    sCdmaCmdCb.pu8AssembleBuf[0] = u8InData;
                    sCdmaCmdCb.u8Cmd             = u8InData;
                    sCdmaCmdCb.u16BufDataLen     = 1;
                    sCdmaCmdCb.eAssembleState    = E_CDMA_RECEIVING_SMS;
                }
            }*/

            break;
        }

        case E_CDMA_RECEIVING_CMD:
        {
            if((u8InData >= 'A') && (u8InData <= 'Z'))
            {
                sCdmaCmdCb.u8Cmd += u8InData;
                sCdmaCmdCb.pu8AssembleBuf[sCdmaCmdCb.u16BufDataLen++] = u8InData;
            }
            else
            {
                if(u8InData == ':')
                {
                    sCdmaCmdCb.pu8AssembleBuf[sCdmaCmdCb.u16BufDataLen++] = u8InData;
                    sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_BODY;
                }
                else
                {
                    sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_HEADER;
                    sCdmaCmdCb.u16BufDataLen  = 0;
                }
            }

            break;
        }

        case E_CDMA_RECEIVING_BODY:
        {
            if(u8InData != '+')
            {
                //�����ݷ������������
                sCdmaCmdCb.pu8AssembleBuf[sCdmaCmdCb.u16BufDataLen++] = u8InData;

                if((u8InData == 'K') && (sCdmaCmdCb.pu8AssembleBuf[sCdmaCmdCb.u16BufDataLen-2] == 'O'))
                {
                    sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_VALID;
                }
                else
                {
                  if((sCdmaCmdCb.u8Cmd == CMD_ZCEND) && (u8InData == 0x0d))
                  {
                      sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_VALID;
                  }
                }
            }
            else
            {
                if(u8InData == '+')
                {
                    Uart0InsertByte('+');
                    sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_VALID;
                }
            }
            break;
        }
/*
        case E_CDMA_RECEIVING_SMS:
        {
            sCdmaCmdCb.pu8AssembleBuf[sCdmaCmdCb.u16BufDataLen++] = u8InData;

            if((u8InData >= 'A') && (u8InData <= 'Z'))
            {
                sCdmaCmdCb.u8Cmd += u8InData;
            }

            if(u8InData == 0x0d)
            {
                sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_VALID;
            }

            break;
        }*/

        case E_CDMA_RECEIVING_VALID:
        {
            break;
        }

        default:
        {
            sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_HEADER;
            sCdmaCmdCb.u16BufDataLen  = 0;
            break;
        }
    }
}

/*****************************************************************************
 �� �� ��  : RegisteUnReadMsg
 ��������  : ��ʶδ�����ŵ�ע�Ỻ����

 �������  : ��
 �� �� ֵ  : BOOL
*****************************************************************************/
BOOL RegisteUnReadMsg(void)
{
    UINT8 *pRxBuf;
    UINT8  i = 0;
    UINT8  u8Temp = 0;

    //��ȡ���õ��ڴ浥Ԫ
    pRxBuf = OSMemGet(pSmallMem, &err);
    memcpy(pRxBuf, sCdmaCmdCb.pu8AssembleBuf, sCdmaCmdCb.u16BufDataLen);

    while((pRxBuf[i] != '"') && (i < sCdmaCmdCb.u16BufDataLen))
    {
        i++;
    }

    if((pRxBuf[i+1] == 'M') && (pRxBuf[i+2] == 'T'))
    {
        if((pRxBuf[i+6] >= '0') && (pRxBuf[i+6] <= '9'))
        {
            if((pRxBuf[i+5] >= '0') && (pRxBuf[i+5] <= '9'))
            {
                u8Temp  = (pRxBuf[i+5] - '0') * 10;
                u8Temp += (pRxBuf[i+6] - '0');
            }
            else
            {
                u8Temp = 0xff;
            }
        }
        else
        {
            if((pRxBuf[i+5] >= '0') && (pRxBuf[i+5] <= '9'))
            {
                u8Temp  = (pRxBuf[i+5] - '0');
            }
            else
            {
                u8Temp = 0xff;
            }
        }
    }
    else
    {
        u8Temp = 0xff;
    }

    //�ͷ��ڴ�
    OSMemPut(pSmallMem, pRxBuf);

    if(u8Temp < MSG_MAX_SIZE)
    {
        au8MsgUnRead[u8Temp] = 1;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/******************************************************************************
 * �������ƣ�GetUnReadMsg
 * ������    �õ�һ��δ���Ķ���Ϣ���
 *
 * ��������
 * ���أ�δ�����ű��
 *****************************************************************************/
SINT8 GetUnReadMsg(void)
{
  SINT8 i;

  for(i=0; i<MSG_MAX_SIZE; i++)
  {
    if(au8MsgUnRead[i] == 1)
    {
      au8MsgUnRead[i] = 0;
      return i;
    }
  }

  return -1;
}

/*****************************************************************************
 �� �� ��  : GetCdmaCsqValue
 ��������  : ������������л�ȡCDMA�ź�ǿ��ֵ

 �������  : none
 �� �� ֵ  : �ź�ǿ��,0~31
*****************************************************************************/
SINT8 GetCdmaCsqValue(void)
{
    SINT8 s8Value=-1;   //CSQֵ
    UINT8 au8Buf[24];
    UINT8 i=0;

    //��ȡ�������е�����
    memcpy(&au8Buf[0], sCdmaCmdCb.pu8AssembleBuf, sCdmaCmdCb.u16BufDataLen);

    while((au8Buf[i] != ':') && (i < sCdmaCmdCb.u16BufDataLen))
    {
        i++;
    }

    if((au8Buf[i+1] == ' ') && (au8Buf[i+2] >= '0') && (au8Buf[i+2] <= '9'))
    {
        i += 2;
        s8Value  = (au8Buf[i] - '0') * 10;
        s8Value += (au8Buf[i+1] - '0');

        if(s8Value > 31)
        {
            return -1;
        }
        else
        {
            return s8Value;
        }
    }
    else
    {
        return -1;
    }

}

/*****************************************************************************
 �� �� ��  : SendReadMsgCmd
 ��������  : ��CDMAģ�鷢�Ͷ�ȡ��������

 �������  : ��
 �� �� ֵ  : BOOL
*****************************************************************************/
SINT8 SendReadMsgCmd(void)
{
    UINT8 u8Buf[24];
    UINT8 i=0;
    SINT8 s8MsgIndex = -1;

    s8MsgIndex = GetUnReadMsg();
    if(s8MsgIndex < 0)
    {
        return -1;
    }

    s8CurrentIndex = s8MsgIndex;
    memset(&u8Buf[0], 0, 24);
    memcpy(&u8Buf[0], &au8CMGR[0], sizeof(au8CMGR));
    i = sizeof(au8CMGD)-1;

    if(s8MsgIndex > 9)
    {
        u8Buf[i]  = s8MsgIndex / 10;
        u8Buf[i] += '0';
        i++;
        u8Buf[i]  = s8MsgIndex % 10;
        u8Buf[i] += '0';
        i++;
        u8Buf[i++] = '\r';

        CdmaSendMsg(&u8Buf[0], i);
    }
    else
    {
        u8Buf[i]  = s8MsgIndex + '0';
        i++;
        u8Buf[i++] = '\r';

        CdmaSendMsg(&u8Buf[0], i);
    }

    return s8MsgIndex;
}

/*****************************************************************************
 �� �� ��  : CdmaCmdHandler
 ��������  : CDMA�����������

 �������  : ��
 �� �� ֵ  : �� , ��
*****************************************************************************/
void CdmaCmdHandler(UINT8 u8Cmd)
{
  SINT8 Csq = -1;

    switch(u8Cmd)
    {
        case CMD_CSQ:
        {
            if(g_bDebug & CDMA_DEBUG)    //��ӡ������Ϣ
            {
                Uart2SendString(" CDMA CSQ Parse Successful!\r\n");
            }

            Csq = GetCdmaCsqValue();
            //�ź�ǿ��С��20�����л�������ģʽ
            //�ٴμ���������´�ϵͳ���Ѻ�
            if(Csq < 20)
            {
                CdmaToBeiDou();
            }

            break;
        }

        case CMD_ZCANS:
        {
          // �ر��ź�ǿ�Ȼ�ȡ��ʱ������ֹ�ڽ�����Ϣʱ�����źŻ�ȡ���»�ȡʧ��
            SwdDisable(sCdmaCmdCb.sCsqDog);
            sCdmaCmdCb.u8MsgEnvent |= BIT0;
            if(g_bDebug & CDMA_DEBUG)    //��ӡ������Ϣ
            {
                Uart2SendString(" CDMA ZCANS Parse Successful!\r\n");
            }
            break;
        }

        case CMD_ZCCNT:
        {
            SwdDisable(sCdmaCmdCb.sCsqDog);
            sCdmaCmdCb.u8MsgEnvent |= BIT1;
            if(g_bDebug & CDMA_DEBUG)    //��ӡ������Ϣ
            {
                Uart2SendString(" CDMA ZCCNT Parse Successful!\r\n");
            }
            break;
        }

        case CMD_CMTI:
        {
            SwdDisable(sCdmaCmdCb.sCsqDog);
            RegisteUnReadMsg();
            if(g_bDebug & CDMA_DEBUG)    //��ӡ������Ϣ
            {
                Uart2SendString(" CDMA CMTI Parse Successful!\r\n");
            }
            break;
        }

        case CMD_ZCEND:
        {
            SwdDisable(sCdmaCmdCb.sCsqDog);
            sCdmaCmdCb.u8MsgEnvent |= BIT3;
            if(g_bDebug & CDMA_DEBUG)    //��ӡ������Ϣ
            {
                Uart2SendString(" CDMA ZCEND Parse Successful!\r\n");
            }
            break;
        }

        case CMD_SMS:
        {
            SwdDisable(sCdmaCmdCb.sCsqDog);
            sCdmaCmdCb.u8MsgEnvent = 0;
            if(g_bDebug & CDMA_DEBUG)    //��ӡ������Ϣ
            {
                Uart2SendString(" CDMA SMS READY!\r\n");
            }
            break;
        }

      case CMD_CMGR:
      {
          SwdEnable(sCdmaCmdCb.sCsqDog);

          //����һ����Ϣɾ��һ����
          DeletCdmaMsg(s8CurrentIndex);
          s8CurrentIndex = -1;
          if(g_bDebug & CDMA_DEBUG)    //��ӡ������Ϣ
          {
            Uart2SendString(" CDMA CMGR Parse Successful!\r\n");
          }
          break;
      }

        default: break;
    }
}

/*****************************************************************************
 �� �� ��  : CdmaCmdRun
 ��������  : CDMA���ݽ���

 �������  : ��
 �� �� ֵ  : �� , ��
*****************************************************************************/
void CdmaCmdRun(void)
{
  if(g_bCdmaSend == TRUE)
  {
    //�Ӵ��ڻ������ڻ�ȡ���ݣ�������н���
    //��Ҫ��Զ��Ž���������̺�CSQ�ź�ǿ�ȵĽ��ա�
    while((sCdmaCmdCb.eAssembleState != E_CDMA_RECEIVING_VALID) && (Uart0GetRxDataLength() > 0))
    {
        CdmaCmdAssemble(Uart0GetByte());

        if(SwdIsEnable(sCdmaCmdCb.sCdmaDog) == TRUE)
        {
            SwdDisable(sCdmaCmdCb.sCdmaDog);
        }
    }

    //�����Ч���������������������
    if(sCdmaCmdCb.eAssembleState == E_CDMA_RECEIVING_VALID)
    {
        if(CMD_ASSERT(sCdmaCmdCb.u8Cmd))
        {
            CdmaCmdHandler(sCdmaCmdCb.u8Cmd);

            if(g_bDebug & CDMA_DEBUG)    //��ӡ������Ϣ
            {
               Uart2SendBuffer(sCdmaCmdCb.pu8AssembleBuf, sCdmaCmdCb.u16BufDataLen);
            }
         }

        memset(sCdmaCmdCb.pu8AssembleBuf, 0, sCdmaCmdCb.u16BufDataLen);
        sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_HEADER;
        sCdmaCmdCb.u16BufDataLen = 0;
    }
    else
    {
       if(SwdIsEnable(sCdmaCmdCb.sCdmaDog) == TRUE)
       {
           if(SwdGetLifeTime(sCdmaCmdCb.sCdmaDog) == 0)
           {
                sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_HEADER;
                sCdmaCmdCb.u16BufDataLen  = 0;

                SwdReset(sCdmaCmdCb.sCdmaDog);
                SwdDisable(sCdmaCmdCb.sCdmaDog);
           }
       }
       else
       {
           if(sCdmaCmdCb.u16BufDataLen != 0)
           {
                SwdReset(sCdmaCmdCb.sCdmaDog);
                SwdEnable(sCdmaCmdCb.sCdmaDog);
           }
       }
    }

    if(SwdGetLifeTime(sCdmaCmdCb.sCsqDog) == 0)
    {
        SwdReset(sCdmaCmdCb.sCsqDog);

        CdmaSendMsg(au8CSQ, sizeof(au8CSQ));
    }

    SendReadMsgCmd();
  }
}

