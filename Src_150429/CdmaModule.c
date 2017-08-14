
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

//ǰ���ǿ������ת��ȱʡ����ɼ������
#define _GetWordFromAddr(pAddr)         (UINT16)(((UINT16)pAddr[0]) << 8) + pAddr[1]

#define _GetDoubleWordFromAddr(pAddr)   ((UINT32)(((UINT32)pAddr[0]) << 24) +    \
                                         (UINT32)(((UINT32)pAddr[1]) << 16) +    \
                                         (UINT32)(((UINT32)pAddr[2]) << 8) + pAddr[3])

//����DTU����ģʽ
const UINT8 u8ConfigMode[] = {"enter_config_mode\0"};
const UINT8 u8ConfigStart[] = {0xff,0xff,0xff,0xff};
const UINT8 u8ConfigRxData[] = {"aaaaaaaaaaaaaaaa\0"};

const UINT8 au8BackOk[3] = {0x4f, 0x4b, 0x00};

//��ѯģ���ͺ�
const UINT8 au8GetModuleIndex[] = {"AT+CGMM\r"};
//���ڲ���������
const UINT8 au8UartSet[] = {"AT+UART=9600,ON,8,N,1\r"};
//����32khz���˯��ģʽ
const UINT8 au8EnableZDSLEEP[] = {"AT+ZDSLEEP=1\r"};
const UINT8 au8DisableZDSLEEP[] = {"AT+ZDSLEEP=0\r"};
//DTR�������߿���
const UINT8 au8EnableZDSLEEPDTR[] = {"AT+ZDSLEEPDTR=1\r"};
const UINT8 au8DisableZDSLEEPDTR[] = {"AT+ZDSLEEPDTR=0\r"};
//��ȡCDMAʱ��
const UINT8 au8GetCdmaTime[] = {"AT+ZDSLEEPDTR=2\r"};
//��λģ��
const UINT8 au8DisableZRST[] = {"AT+ZRST=0\r"};
//����λ,1���Ӻ�λ
//const UINT8 au8EnableZRST[] = {"AT+ZRST=1,"00:01"\r"};

//�ź�ǿ�Ȳ�ѯ�����أ�+CSQ: 28,99 OK��16���¿��ܺ���ʧ�ܡ�
const UINT8 au8CSQ[] = {"AT+CSQ\r"};
//ʱ�Ӳ�ѯ ���أ�+CCLK:"2004/02/09��17��34��23.694"��ǰ����ʱ�䡣
const UINT8 au8CCLK[] = {"AT+CCLK?\r"};

//���ñ�������
const UINT8 au8SetCNUM[] = {"AT+CNUM="};
const UINT8 au8GetCNUM[] = {"AT+CNUM?"};

//ģ��ָʾ
const UINT8 au8ZIND[] = {"AT+ZIND="};

/////////////////////////////////////////////////////////////
// ���ŷ���
//ѡ����ŷ���
const UINT8 au8MsgType[] = {"AT+CSMS?\r"};
//���ö���ΪӢ�ģ� ascii ��ʽ
const UINT8 au8ZMSGL_ASCII[] = {"AT+ZMSGL=1,2\r"};
//���ö���Ϊ���� �� unicode��ʽ
const UINT8 au8ZMSCL_UNICODE[] = {"AT+ZMSGL=6,4\r"};
//���Ͷ�Ϣ
typedef struct _tsCMGS
{
  UINT8 *pCmd; //���AT���� AT+CMGS=
  
  UINT8 *pPhoneNum;  //��ŵ绰���룬��ǰ��ġ���
  
  UINT8 * au8Info;    //�����������Ϣָ��

}tsCMGS, *pCMGS;


/*****************************************************************************
 �� �� ��  : CdmaSendMsg
 ��������  : CDMAģ�鷢�Ͷ���

 �������  : none
 �� �� ֵ  : none,
*****************************************************************************/
BOOL CdmaSendMsg(const UINT8 * ptr, UINT8 u8Len)
{
  Uart0SendBuffer(ptr, u8Len);
  return 1;
}

/*****************************************************************************
 �� �� ��  : AssembleCdmaMsg
 ��������  : AssembleCdmaMsg

 �������  : ������Ϣָ��
 �� �� ֵ  : none,
*****************************************************************************/
void AssembleCdmaMsg(UINT8 *pBuf, UINT8 u8Len)
{
  UINT8 *pMsg;
  UINT8 i=0;
  
  pMsg = OSMemGet(pSmallMem, &err);
//  memset(pMsg, 0, BUF_SMALL_SIZE);
  
  memcpy(&pMsg[i], "AT+CMGS=", 8);
  i += 8; 
  pMsg[i++] = '"';
  memcpy(&pMsg[i], "18561795199", 11);
  i += 11;
  pMsg[i++] = '"';
  pMsg[i++] = '\r';
  
  memcpy(&pMsg[i], pBuf, u8Len);
  i += u8Len;
  
  pMsg[i++] = 0x1A;   //������
  
  CdmaSendMsg(pMsg, i);

  //�ͷ��ڴ�
  OSMemPut(pSmallMem, pMsg);
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
  
 	//CDMA RI�жϣ��½��ش����ж�
 	CdmaRiIesEdge();
 	CdmaRiIntEnable();
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
  while(g_u16TimerDelay < 200);  //��ʱ2s
  SetCdmaOnOff(1);
  
  CdmaSendMsg(au8ZMSGL_ASCII, sizeof(au8ZMSGL_ASCII));
  
}

/*****************************************************************************
 �� �� ��  : GetCdmaModule
 ��������  : CDMAģ���ʼ��

 �������  : none
 �� �� ֵ  : none,
*****************************************************************************/
void GetCdmaModule(void)
{
  UINT8 i;
  SINT8 sDog;
  UINT8 *pRxBuf;
 
  pRxBuf = OSMemGet(pSmallMem, &err);
  memset(pRxBuf, 0, BUF_SMALL_SIZE);

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);
  
  //��մ��ڻ�����
  Uart0EmptyRxBuf();
  //����ģ���ͺ�����
  Uart0SendBuffer(au8GetModuleIndex, sizeof(au8GetModuleIndex));
  
  while(TRUE)
  {
    if(Uart0GetRxDataLength() > 0)
    {
      pRxBuf[i++] = Uart0GetByte();
      if((pRxBuf[i-1] == 'O') && (pRxBuf[i] == 'K'))
      {
        break;
      }
    }
    
    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);
      i = 0;
      
      //��մ��ڻ�����
      Uart0EmptyRxBuf();
      //����ģ���ͺ�����
      Uart0SendBuffer(au8GetModuleIndex, sizeof(au8GetModuleIndex));
    } 
  }
  
  if(g_bDebug == TRUE)
  {
    if(strstr(pRxBuf, "MC8332") != NULL)
    {
      Uart3SendString("#Detect ZTE CDMA Module. The Product Model Is MC8332!\r\n");
    }
    else
    {
      Uart3SendString("#CDMA Module To Find The Failure!\r\n");
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
  UINT8 i;
  SINT8 sDog;
  UINT8 *pRxBuf;
  UINT8 u8Temp;
  
  pRxBuf = OSMemGet(pSmallMem, &err);
  memset(pRxBuf, 0, BUF_SMALL_SIZE);

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);
  //��մ��ڻ�����
  Uart0EmptyRxBuf();
  //����ģ���ͺ�����
  Uart0SendBuffer(au8CSQ, sizeof(au8CSQ));
  
  while(TRUE)
  {
    if(Uart0GetRxDataLength() > 0)
    {
      pRxBuf[i++] = Uart0GetByte();
      if((pRxBuf[i-1] == 'O') && (pRxBuf[i] == 'K'))
      {
        i = 0;
        while((pRxBuf[i] != '"') && (i<BUF_SMALL_SIZE))
        {
          i++;
        }
  
        if((pRxBuf[i-2] >= 0x30) && (pRxBuf[i-1] >= 0x30))
        {
          u8Temp = (pRxBuf[i-2]-0x30)*10 + (pRxBuf[i-1]-0x30);
          break;
        }
      }
    }
    
    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);
      i = 0;
      
      //��մ��ڻ�����
      Uart0EmptyRxBuf();
      //����ģ���ͺ�����
      Uart0SendBuffer(au8CSQ, sizeof(au8CSQ));
    } 
  }
    
  //�ͷ��ڴ�
  OSMemPut(pSmallMem, pRxBuf);
  //�ͷ������ʱ��
  SwdFree(sDog);
  
  return u8Temp;
}


/*****************************************************************************
 �� �� ��  : SetCdmaTimeToRtc
 ��������  : ��ȡCDMAʱ��, ���õ�RTC

 �������  : none
 �� �� ֵ  : TRUE -- У׼�ɹ��� FALSE -- У׼ʧ��
*****************************************************************************/
BOOL SetCdmaTimeToRtc(void)
{
  UINT8 i,j;
  SINT8 sDog;
  UINT8 *pRxBuf;
  UINT8 au8Time[22];
  _tsTimeCb sTime;
  
  pRxBuf = OSMemGet(pSmallMem, &err);
  memset(pRxBuf, 0, BUF_SMALL_SIZE);

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);
  //��մ��ڻ�����
  Uart0EmptyRxBuf();
  //����ģ���ͺ�����
  Uart0SendBuffer(au8CCLK, sizeof(au8CCLK));
  
  while(TRUE)
  {
    if(Uart0GetRxDataLength() > 48)
    {
      Uart0GetRxData(pRxBuf, 48);

      i = 0;
      while((pRxBuf[i] != '"') && (i<BUF_SMALL_SIZE))
      {
        i++;
      }
  
      j = ++i;
      while((pRxBuf[j] != '"') && (j<BUF_SMALL_SIZE))
      {
        j++;
      }
        
      break;
    }
    
    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);
      i = 0;
      
      //��մ��ڻ�����
      Uart0EmptyRxBuf();
      //����ģ���ͺ�����
      Uart0SendBuffer(au8CCLK, sizeof(au8CCLK));
    } 
  }
  //ʱ�俽��������ms����
  memcpy(au8Time, &pRxBuf[i], j-i);
  //�ͷ��ڴ�
  OSMemPut(pSmallMem, pRxBuf);
  //�ͷ������ʱ��
  SwdFree(sDog);
  
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
  
//  SetRtcInt1Out();
  SetTimeToRtc(&sTime);
  
  return TRUE;
}