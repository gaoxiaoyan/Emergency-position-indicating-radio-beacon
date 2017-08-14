
/******************************************************************************
*											
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved										
*
* �� �� ���� DtuModule.C
* ��    ���� DTU���ݽ���ģ��

* �������ڣ� 2014��9��5��11:31:59
* ��    �ߣ� Bob
* ��ǰ�汾�� V1.00

------------------------------------------------------
*******************************************************************************/

#ifdef MSP430F149
#include "msp430x14x.h"
#else
#include "msp430x54x.h"
#endif

#include "DtuModule.h"
#include "DataType.h"
#include "Uart.h"
#include "SystemCtl.h"
#include "string.h"
#include "Flash.h"
#include "Timer.h"
#include "GpsParse.h"
#include "UartCmd.h"
#include "Display.h"
#include "stdio.h"

//ǰ���ǿ������ת��ȱʡ����ɼ������
#define _GetWordFromAddr(pAddr)         (UINT16)(((UINT16)pAddr[0]) << 8) + pAddr[1]

#define _GetDoubleWordFromAddr(pAddr)   ((UINT32)(((UINT32)pAddr[0]) << 24) +    \
                                         (UINT32)(((UINT32)pAddr[1]) << 16) +    \
                                         (UINT32)(((UINT32)pAddr[2]) << 8) + pAddr[3])



//��������ṹ����
_tsUartCmdCb sDtuCmdCb;

//DTU���ջ�����
UINT8 au8DtuBodyBuf[MAX_BODY_SIZE];

//TCP / UDP
#define TCP         0
#define UDP         1

//�ƶ� / ��ͨѡ��
#define CMNET       0
#define UNINET      1

//����DTU����ģʽ
const UINT8 u8ConfigMode[] = {"enter_config_mode\0"};
const UINT8 u8ConfigStart[] = {0xff,0xff,0xff,0xff};
const UINT8 u8ConfigRxData[] = {"aaaaaaaaaaaaaaaa\0"};

const UINT8 au8BackOk[3] = {0x4f, 0x4b, 0x00};

//���ڲ���������
const UINT8 au8UartSet[] = {"AT+UART=9600,ON,8,N,1\r"};
//ʹ�ù̶���Э�飬ͨѶ��ʽ���û��������롣
const UINT8 au8Tcp[] = {"AT+NET="}; //TCP,CMNET,card,card,"};
//�ϵ��Զ�����
const UINT8 au8AutoCon[] = {"AT+AUTOCON=1\r"};
//�豸ID
const UINT8 au8Id[] = {"AT+ID="};
const UINT8 au8GetId[] = {"AT+ID?\r"};
//�����ز�ʱ��(��)
const UINT8 au8RedialTime[] = {"AT+REDIALTIME=8\r"};
//�������ʱ�䣨���ӣ�
const UINT8 au8TcpIdle[] = {"AT+TCPIDLE=30\r"};
//����������
const UINT8 au8Beat[] = {"AT+BEAT=30,KYTENCE,0\r"};
//���÷������ݰ����
const UINT8 au8PkgTime[] = {"AT+PKGT=100\r"};
//����DSC
const UINT8 au8Dsc[] = {"AT+DSC=1\r"};
//������ģʽ
const UINT8 au8NetMode[] = {"AT+NETMODE=1\r"};
//����ģʽ����
const UINT8 au8Trace[] = {"AT+TRACE=0\r"};
//���͸�λ����
const UINT8 au8Reset[] = {"AT+RESET=1\r"};

//������1����
const UINT8 au8EnableSub1[] = {"AT+NET1=1,"};
const UINT8 au8DisableSub1[] = {"AT+NET1=0,"};
//������2����
const UINT8 au8EnableSub2[] = {"AT+NET2=1,"};
const UINT8 au8DisableSub2[] = {"AT+NET2=0,"};
//������3����
const UINT8 au8EnableSub3[] = {"AT+NET3=1,"};
const UINT8 au8DisableSub3[] = {"AT+NET3=0,"};
//������4����
const UINT8 au8EnableSub4[] = {"AT+NET4=1,"};
const UINT8 au8DisableSub4[] = {"AT+NET4=0,"};

#if 1
/*****************************************************************************
 �� �� ��  : Delay
 ��������  : ��12MHz���� 1T�� ��ʱʱ��Լ����usTime ms
 �������  : USHORT16 usTime
 �������  : NONE
 �� �� ֵ  :
*****************************************************************************/
static void Delay(UINT16 usTime)
{
    UINT16 j;
    UINT32 i;

    for (i = usTime; i > 0; i--)
    {
        for(j = 110; j > 0; j--);
    }
}


/*****************************************************************************
 �� �� ��  : DtuEnterConfigMode
 ��������  : DTU module initialize, enter config mode

 �������  : none
 �� �� ֵ  : ����״̬,
*****************************************************************************/
UINT8 DtuEnterConfigMode(void)
{
  UINT8 u8Buf[20];
  UINT8 u8State = 0;
  UINT8 i;
  
  MEM_SET(u8Buf, 0, 20);
  DtuPowerOff();
  Delay(10000);
  DtuPowerOn();
//  Delay(100);
  
  //������0���ճ�ʱ��ʱ����ΪDTU����ʱ�ĳ�ʱ��ʱ������ʱΪ10��
  asTimer[TIMER_UART1].u16LifeTime = TIMER_TICK * 10;
  asTimer[TIMER_UART1].u16LifeOldTime = TIMER_TICK * 10;
  asTimer[TIMER_UART1].bEnable = TRUE;
  
  while(TRUE)
  {
    FreeWatchDog();         //�幷,����Ϊ16s
    
    switch(u8State)
    {
      case 0:
      {  
        for(i=0; i<255; i++)
        {
          UartSendBuffer(UART1, u8ConfigStart, sizeof(u8ConfigStart));
          
          if(Uart1GetRxDataLength() > 16)
          {
            u8State = 1;
            break;
          }
        }

        break;
      }
      
      case 1:
      {
        Uart1GetRxData(u8Buf, 17);         
        if(0 == MEM_CMP(u8Buf, u8ConfigRxData, 17))
        {
          MEM_SET(u8Buf, 0, 20);
          u8State = 2;
        }
        else
        {
          u8State = 0;
        }

        break;
      }
      
      case 2:
      {
        if(Uart1GetRxDataLength() > 17)
        {
          Uart1GetRxData(u8Buf, 18);
          if(0 == MEM_CMP(u8Buf, u8ConfigMode, 18))
          {
            u8State = 3;
          }
          else
          {
            u8State = 0;
          }
        }
        break;
      }
      
      case 3:
      {
        //�ָ�����0���ճ�ʱ��ʱ����
        asTimer[TIMER_UART1].u16LifeTime = TIMER_TICK / 2;
        asTimer[TIMER_UART1].u16LifeOldTime = TIMER_TICK / 2;
        asTimer[TIMER_UART1].bEnable = FALSE;
        
        return DTU_CFG_MODE;
      }
      
      default: u8State = 0;
    }
    
    if(asTimer[TIMER_UART1].u16LifeTime == 0)
    {
      //�ָ�����0���ճ�ʱ��ʱ����
      asTimer[TIMER_UART1].u16LifeTime = TIMER_TICK / 2;
      asTimer[TIMER_UART1].u16LifeOldTime = TIMER_TICK / 2;
      asTimer[TIMER_UART1].bEnable = FALSE;
        
      return DTU_CFG_ERROR;
    }
    
  }
}

/*****************************************************************************
 �� �� ��  : GetDtuConfigInfo
 ��������  : Get DTU module config, Here mainly get module ID.

 �������  : none
 �� �� ֵ  : ����״̬
*****************************************************************************/
UINT8 GetDtuConfigInfo(UINT8 *pu8Buf)
{
  UINT8 u8CfgState=0xff;
  UINT8 i,j,u8Temp;
  UINT8 au8IdBuf[16]={0};
  
  //��DTU��������ģʽ
  u8CfgState = DtuEnterConfigMode();
  if(DTU_CFG_MODE == u8CfgState)
  {
    //���BUF1
    Uart1EmptyRxBuf();
     //����TCP
    UartSendBuffer(UART1, &au8GetId[0], (sizeof(au8GetId)-1));
    while(Uart1GetRxDataLength() <22);
    for(i=0; i<10; i++)
      Uart1GetByte();
      
    for(i=0;i<16;i++)
    {
      u8Temp = Uart1GetByte();
      if(u8Temp != ';')
      {
        pu8Buf[i] = u8Temp;
      }
      else
      {
        break;
      }
    }

    //����λ����
     Delay(1000);
    UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
    Delay(1000);
    return DTU_CFG_OK;
  }
    else
  {
    //����λ����
    UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
    
    return DTU_CFG_ERROR;
  }
}

/*****************************************************************************
 �� �� ��  : GetStringLen
 ��������  : ��ȡ�ַ������ȣ���0����

 �������  : *S�ַ���ָ��
 �� �� ֵ  : �ַ�������
*****************************************************************************/
UINT8 GetStringLen(UINT8 *S)
{
  UINT8 i=0;
  
  while(S[i] != 0)
  {
    i++;
  }
  
  if(i>0)
    return i;
  else
    return 0;
}

UINT8 GetDataWidth(UINT32 u32Data)
{ 
  if(u32Data < 10)
    return 1;
  else if((u32Data > 9) && (u32Data < 100))
    return 2;
  else if((u32Data > 99) && (u32Data < 1000))
    return 3;
  else if((u32Data > 999) && (u32Data < 10000))
    return 4;
  else if((u32Data > 9999) && (u32Data < 100000))
    return 5;
  else
    return 6;
}


/*****************************************************************************
 �� �� ��  : ConfigDtuModule
 ��������  : Config dtu module, 

 �������  : UINT8 u8State
 �� �� ֵ  :
*****************************************************************************/
UINT8 McuConfigDtuMainServer(void)
{
  UINT8 u8StrLen, u8IpLen; //�ַ�������  
  UINT8 au8ServerBuf[128];
  char au8IpBuf[64];
  UINT16 u16CfgCom;
  UINT8 u8CfgState = 0xff;
  
  MEM_SET(au8ServerBuf, 0, sizeof(au8ServerBuf));
  u8StrLen = 7; //GetStringLen(&au8Tcp[0]);
  MEM_CPY(&au8ServerBuf[0], &au8Tcp[0], u8StrLen);
  
  //TCP/UDPЭ��
  if(sMainServer.bBckTcpUdp == TCP)
  {
    MEM_CPY(&au8ServerBuf[u8StrLen], "TCP,", 4);
  }
  else
  {
    if(sMainServer.bBckTcpUdp == UDP)
    {
      MEM_CPY(&au8ServerBuf[u8StrLen], "UDP,", 4);
    }
  }
  
  u8StrLen += 4;
  
  //�������
  if(sMainServer.bCfgApn == CMNET)    //�й��ƶ�
  {
    MEM_CPY(&au8ServerBuf[u8StrLen], "CMNET,", 6);
    u8StrLen += 6;
  }
  else
  {
    if(sMainServer.bCfgApn == UNINET) //�й���ͨ
    {
      MEM_CPY(&au8ServerBuf[u8StrLen], "UNINET,", 7);
      u8StrLen += 7;
    }
  }
  
  if((sMainServer.u8CfgUserNameLen == 1) && (sMainServer.u8CfgPasswordLen == 1))
  {
    au8ServerBuf[u8StrLen++] = ',';
    au8ServerBuf[u8StrLen++] = ',';
  }
  else
  {
    //�û�������
    MEM_CPY(&au8ServerBuf[u8StrLen], &sMainServer.au8CfgUserName[0], sMainServer.u8CfgUserNameLen);
    u8StrLen += sMainServer.u8CfgUserNameLen;
    au8ServerBuf[u8StrLen++] = ',';
    //�û���������
    MEM_CPY(&au8ServerBuf[u8StrLen], &sMainServer.au8CfgPassword[0], sMainServer.u8CfgPasswordLen);
    u8StrLen += sMainServer.u8CfgPasswordLen;
    au8ServerBuf[u8StrLen++] = ',';
  }
  
  //IP��ַ��ȡ
  if(sMainServer.u8CfgIpLen == 4)
  {
    u8IpLen = 0;
    MEM_SET(&au8IpBuf[0], 0, sizeof(au8IpBuf));
    sprintf(&au8IpBuf[u8IpLen],"%d", sMainServer.au8CfgIpAddr[0]);
    u8IpLen += GetDataWidth(sMainServer.au8CfgIpAddr[0]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sMainServer.au8CfgIpAddr[1]);
    u8IpLen += GetDataWidth(sMainServer.au8CfgIpAddr[1]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sMainServer.au8CfgIpAddr[2]);
    u8IpLen += GetDataWidth(sMainServer.au8CfgIpAddr[2]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sMainServer.au8CfgIpAddr[3]);
    u8IpLen += GetDataWidth(sMainServer.au8CfgIpAddr[3]);
    au8IpBuf[u8IpLen++] = ',';
    
    MEM_CPY(&au8ServerBuf[u8StrLen], &au8IpBuf[0], u8IpLen);
    
    u8StrLen += u8IpLen;
  }
  else
  {
    MEM_CPY(&au8ServerBuf[u8StrLen], &sMainServer.au8CfgIpAddr[0], sMainServer.u8CfgIpLen);
    u8StrLen += sMainServer.u8CfgIpLen;
    au8ServerBuf[u8StrLen++] = ',';
  }

  //���ö˿ں�
  u16CfgCom = (UINT16)(((UINT16)sMainServer.au8CfgComIndex[0]) << 8) + sMainServer.au8CfgComIndex[1];
  MEM_SET(&au8IpBuf[0], 0, sizeof(au8IpBuf));
  sprintf(&au8IpBuf[0], "%d", u16CfgCom);
  u8IpLen = GetDataWidth(u16CfgCom);
  MEM_CPY(&au8ServerBuf[u8StrLen], &au8IpBuf[0], u8IpLen);
  u8StrLen += u8IpLen;
  au8ServerBuf[u8StrLen++] = '\r';
  
  //��DTU��������ģʽ
  u8CfgState = DtuEnterConfigMode();
  if(DTU_CFG_MODE == u8CfgState)
  {
    //���BUF1
    Uart1EmptyRxBuf();
    //����UART
    UartSendBuffer(UART1, &au8UartSet[0], (sizeof(au8UartSet)-1));
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //����λ����
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_UART_SET_ERROR;
      }
    }
    
    //����TCP
    UartSendBuffer(UART1, &au8ServerBuf[0], u8StrLen);
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //����λ����
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_TCP_SET_ERROR;
      }
    }
    
    //�����Զ�������
    UartSendBuffer(UART1, &au8AutoCon[0], (sizeof(au8AutoCon)-1));
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //����λ����
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_AUTO_SET_ERROR;
      }
    }
    
    //ID����
    u8StrLen = 0;
    u8IpLen = sizeof(au8Id)-1;
    MEM_CPY(&au8IpBuf[u8StrLen], &au8Id[0],  u8IpLen);
    u8StrLen += u8IpLen;
    MEM_CPY(&au8IpBuf[u8StrLen], &sMainServer.au8CfgId[0], 11);
    u8StrLen += 11;
    au8IpBuf[u8StrLen++] = ',';
    au8IpBuf[u8StrLen++] = '0';
    au8IpBuf[u8StrLen++] = '\r';
    
    UartSendBuffer(UART1, &au8IpBuf[0], u8StrLen);
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //����λ����
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_ID_SET_ERROR;
      }
    }
    
    //�����ز�ʱ��
    UartSendBuffer(UART1, &au8RedialTime[0], (sizeof(au8RedialTime)-1));
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //����λ����
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_AGAIN_SET_ERROR;
      }
    }
    
    //�����������ʱ��
    UartSendBuffer(UART1, &au8TcpIdle[0], (sizeof(au8TcpIdle)-1));
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      { 
        //����λ����
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_IDLE_SET_ERROR;
      }
    }
    
    //����������
    UartSendBuffer(UART1, &au8Beat[0], (sizeof(au8Beat)-1));
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //����λ����
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_BEAT_SET_ERROR;
      }
    }
    
    //�������ݰ����
    UartSendBuffer(UART1, &au8PkgTime[0], (sizeof(au8PkgTime)-1));
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //����λ����
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_GAP_SET_ERROR;
      }
    }

    //����DCS
    UartSendBuffer(UART1, &au8Dsc[0], (sizeof(au8Dsc)-1));
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //����λ����
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_DCS_SET_ERROR;
      }
    }
    
    //���ö����ġ�����ģʽ
    UartSendBuffer(UART1, &au8NetMode[0], (sizeof(au8NetMode)-1));
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //����λ����
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_MUL_SET_ERROR;
      }
    }
    
    //���õ��Կ���
    UartSendBuffer(UART1, &au8Trace[0], (sizeof(au8Trace)-1));
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //����λ����
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_DEB_SET_ERROR;
      }
    }
    
    //����λ����
    UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
    g_u8NeedSaved |= SAVE_DTU_MAIN;
    
    return DTU_CFG_OK;
  }
  else
  {
    //����λ����
    UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
    
    return DTU_CFG_ERROR;
  }
}

/*****************************************************************************
 �� �� ��  : McuConfigDtuSub1
 ��������  : Config dtu module, 

 �������  : UINT8 u8State
 �� �� ֵ  :
*****************************************************************************/
UINT8 McuConfigDtuSub1(UINT8 u8CfgEnable)
{
  UINT8  u8IpLen;   
  char  au8IpBuf[64];
  UINT16 u16CfgCom;
  UINT8  u8CfgState = 0xff;
    
  u8IpLen = 0;
  MEM_SET(&au8IpBuf[u8IpLen], 0, sizeof(au8IpBuf));
  if(u8CfgEnable == DTU_CFG_ENABLE)
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &au8EnableSub1[0], (sizeof(au8EnableSub1)-1));
    u8IpLen += sizeof(au8EnableSub1)-1;
  }
  else
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &au8DisableSub1[0], (sizeof(au8DisableSub1)-1));
    u8IpLen += sizeof(au8DisableSub1)-1;
  }
    
  //IP��ַ��ȡ
  if(sDtuCfg[IP_SUB_1].u8CfgIpLen == 4)
  {
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_1].au8CfgIpAddr[0]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_1].au8CfgIpAddr[0]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_1].au8CfgIpAddr[1]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_1].au8CfgIpAddr[1]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_1].au8CfgIpAddr[2]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_1].au8CfgIpAddr[2]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_1].au8CfgIpAddr[3]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_1].au8CfgIpAddr[3]);
    au8IpBuf[u8IpLen++] = ',';
  }
  else
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &sDtuCfg[IP_SUB_1].au8CfgIpAddr[0], sDtuCfg[IP_SUB_1].u8CfgIpLen);
    u8IpLen += sDtuCfg[IP_SUB_1].u8CfgIpLen;
    au8IpBuf[u8IpLen++] = ',';
  }

  //���ö˿ں�
  u16CfgCom = (UINT16)(((UINT16)sDtuCfg[IP_SUB_1].au8CfgComIndex[0]) << 8) + sDtuCfg[IP_SUB_1].au8CfgComIndex[1];
  sprintf(&au8IpBuf[u8IpLen], "%d", u16CfgCom);
  u8IpLen += GetDataWidth(u16CfgCom);
  au8IpBuf[u8IpLen++] = '\r';
  
  //��DTU��������ģʽ
  u8CfgState = DtuEnterConfigMode();
  if(DTU_CFG_MODE == u8CfgState)
  {
    //���BUF1
    Uart1EmptyRxBuf();
    //����UART
    UartSendBuffer(UART1, &au8IpBuf[0], u8IpLen);
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //����λ����
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_IP_SUB1_ERROR;
      }     
      //����λ����
      UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
      g_u8NeedSaved |= SAVE_DTU_SUB1;
        
      return DTU_CFG_OK;
    }
   
    return DTU_CFG_ERROR;
  }
  else
  {
    //����λ����
    UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
    
    return DTU_CFG_ERROR;
  }
  
}

/*****************************************************************************
 �� �� ��  : McuConfigDtuSub2
 ��������  : Config dtu module, 

 �������  : UINT8 u8State
 �� �� ֵ  :
*****************************************************************************/
UINT8 McuConfigDtuSub2(UINT8 u8CfgEnable)
{
  UINT8  u8IpLen;   
  char  au8IpBuf[64];
  UINT16 u16CfgCom;
  UINT8  u8CfgState = 0xff;
    
  u8IpLen = 0;
  MEM_SET(&au8IpBuf[u8IpLen], 0, sizeof(au8IpBuf));
  if(u8CfgEnable == DTU_CFG_ENABLE)
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &au8EnableSub2[0], (sizeof(au8EnableSub1)-1));
    u8IpLen += sizeof(au8EnableSub1)-1;
  }
  else
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &au8DisableSub2[0], (sizeof(au8DisableSub1)-1));
    u8IpLen += sizeof(au8DisableSub1)-1;
  }
    
  //IP��ַ��ȡ
  if(sDtuCfg[IP_SUB_2].u8CfgIpLen == 4)
  {
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_2].au8CfgIpAddr[0]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_2].au8CfgIpAddr[0]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_2].au8CfgIpAddr[1]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_2].au8CfgIpAddr[1]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_2].au8CfgIpAddr[2]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_2].au8CfgIpAddr[2]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_2].au8CfgIpAddr[3]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_2].au8CfgIpAddr[3]);
    au8IpBuf[u8IpLen++] = ',';
  }
  else
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &sDtuCfg[IP_SUB_2].au8CfgIpAddr[0], sDtuCfg[IP_SUB_2].u8CfgIpLen);
    u8IpLen += sDtuCfg[IP_SUB_2].u8CfgIpLen;
    au8IpBuf[u8IpLen++] = ',';
  }

  //���ö˿ں�
  u16CfgCom = (UINT16)(((UINT16)sDtuCfg[IP_SUB_2].au8CfgComIndex[0]) << 8) + sDtuCfg[IP_SUB_2].au8CfgComIndex[1];
  sprintf(&au8IpBuf[u8IpLen], "%d", u16CfgCom);
  u8IpLen += GetDataWidth(u16CfgCom);
  au8IpBuf[u8IpLen++] = '\r';
  
  //��DTU��������ģʽ
  u8CfgState = DtuEnterConfigMode();
  if(DTU_CFG_MODE == u8CfgState)
  {
    //���BUF1
    Uart1EmptyRxBuf();
    //����UART
    UartSendBuffer(UART1, &au8IpBuf[0], u8IpLen);
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //����λ����
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_UART_SET_ERROR;
      }
      
      //����λ����
      UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
      g_u8NeedSaved |= SAVE_DTU_SUB2;
      
      return DTU_CFG_OK;
    }
    
    return DTU_CFG_ERROR;
  }
  else
  {
    //����λ����
    UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
    return DTU_CFG_ERROR;
  }
}

/*****************************************************************************
 �� �� ��  : McuConfigDtuSub3
 ��������  : Config dtu module, 

 �������  : UINT8 u8State
 �� �� ֵ  :
*****************************************************************************/
UINT8 McuConfigDtuSub3(UINT8 u8CfgEnable)
{
  UINT8  u8IpLen;   
  char  au8IpBuf[64];
  UINT16 u16CfgCom;
  UINT8  u8CfgState = 0xff;
    
  u8IpLen = 0;
  MEM_SET(&au8IpBuf[u8IpLen], 0, sizeof(au8IpBuf));
  if(u8CfgEnable == DTU_CFG_ENABLE)
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &au8EnableSub3[0], (sizeof(au8EnableSub1)-1));
    u8IpLen += sizeof(au8EnableSub1)-1;
  }
  else
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &au8DisableSub3[0], (sizeof(au8DisableSub1)-1));
    u8IpLen += sizeof(au8DisableSub1)-1;
  }
    
  //IP��ַ��ȡ
  if(sDtuCfg[IP_SUB_3].u8CfgIpLen == 4)
  {
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_3].au8CfgIpAddr[0]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_3].au8CfgIpAddr[0]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_3].au8CfgIpAddr[1]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_3].au8CfgIpAddr[1]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_3].au8CfgIpAddr[2]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_3].au8CfgIpAddr[2]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_3].au8CfgIpAddr[3]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_3].au8CfgIpAddr[3]);
    au8IpBuf[u8IpLen++] = ',';
  }
  else
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &sDtuCfg[IP_SUB_3].au8CfgIpAddr[0], sDtuCfg[IP_SUB_3].u8CfgIpLen);
    u8IpLen += sDtuCfg[IP_SUB_3].u8CfgIpLen;
    au8IpBuf[u8IpLen++] = ',';
  }

  //���ö˿ں�
  u16CfgCom = (UINT16)(((UINT16)sDtuCfg[IP_SUB_3].au8CfgComIndex[0]) << 8) + sDtuCfg[IP_SUB_3].au8CfgComIndex[1];
  sprintf(&au8IpBuf[u8IpLen], "%d", u16CfgCom);
  u8IpLen += GetDataWidth(u16CfgCom);
  au8IpBuf[u8IpLen++] = '\r';
  
  //��DTU��������ģʽ
  u8CfgState = DtuEnterConfigMode();
  if(DTU_CFG_MODE == u8CfgState)
  {
    //���BUF1
    Uart1EmptyRxBuf();
    //����UART
    UartSendBuffer(UART1, &au8IpBuf[0], u8IpLen);
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //����λ����
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_UART_SET_ERROR;
      }
      
      //����λ����
      UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
      g_u8NeedSaved |= SAVE_DTU_SUB3;
      
      return DTU_CFG_OK;
    }
    
    return DTU_CFG_ERROR;
  }
  else
  {
    //����λ����
    UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
    return DTU_CFG_ERROR;
  }
}

/*****************************************************************************
 �� �� ��  : McuConfigDtuSub1
 ��������  : Config dtu module, 

 �������  : UINT8 u8State
 �� �� ֵ  :
*****************************************************************************/
UINT8 McuConfigDtuSub4(UINT8 u8CfgEnable)
{
  UINT8  u8IpLen;   
  char  au8IpBuf[64];
  UINT16 u16CfgCom;
  UINT8  u8CfgState = 0xff;
    
  u8IpLen = 0;
  MEM_SET(&au8IpBuf[u8IpLen], 0, sizeof(au8IpBuf));
  if(u8CfgEnable == DTU_CFG_ENABLE)
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &au8EnableSub4[0], (sizeof(au8EnableSub1)-1));
    u8IpLen += sizeof(au8EnableSub1)-1;
  }
  else
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &au8DisableSub4[0], (sizeof(au8DisableSub1)-1));
    u8IpLen += sizeof(au8DisableSub1)-1;
  }
    
  //IP��ַ��ȡ
  if(sDtuCfg[IP_SUB_4].u8CfgIpLen == 4)
  {
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_4].au8CfgIpAddr[0]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_4].au8CfgIpAddr[0]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_4].au8CfgIpAddr[1]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_4].au8CfgIpAddr[1]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_4].au8CfgIpAddr[2]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_4].au8CfgIpAddr[2]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_4].au8CfgIpAddr[3]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_4].au8CfgIpAddr[3]);
    au8IpBuf[u8IpLen++] = ',';
  }
  else
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &sDtuCfg[IP_SUB_4].au8CfgIpAddr[0], sDtuCfg[IP_SUB_4].u8CfgIpLen);
    u8IpLen += sDtuCfg[IP_SUB_4].u8CfgIpLen;
    au8IpBuf[u8IpLen++] = ',';
  }

  //���ö˿ں�
  u16CfgCom = (UINT16)(((UINT16)sDtuCfg[IP_SUB_4].au8CfgComIndex[0]) << 8) + sDtuCfg[IP_SUB_4].au8CfgComIndex[1];
  sprintf(&au8IpBuf[u8IpLen], "%d", u16CfgCom);
  u8IpLen += GetDataWidth(u16CfgCom);
  au8IpBuf[u8IpLen++] = '\r';
  
  //��DTU��������ģʽ
  u8CfgState = DtuEnterConfigMode();
  if(DTU_CFG_MODE == u8CfgState)
  {
    //���BUF1
    Uart1EmptyRxBuf();
    //����UART
    UartSendBuffer(UART1, &au8IpBuf[0], u8IpLen);
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //����λ����
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_UART_SET_ERROR;
      }
      
      //����λ����
      UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
      g_u8NeedSaved |= SAVE_DTU_SUB4;
      
      return DTU_CFG_OK;
    }
    
    return DTU_CFG_ERROR;
  }
  else
  {
    //����λ����
    UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
    return DTU_CFG_ERROR;
  }
}



  

/*****************************************************************************
 �� �� ��  : SendTargetInfo
 ��������  : Send infomation to center through DTU module

 �������  : USHORT16 usTime
 �� �� ֵ  :
*****************************************************************************/
void SendTargetInfo(void)
{
  UINT8 j;
  UINT8 i=0;
  UINT8 au8Data[120];
  UINT8 u8CheckSum=0;
  
  MEM_SET(au8Data, 0, 120);
  
  //����֡ͷ
  au8Data[i++] = '$';

  //ָ���� -- 4byte
  au8Data[i++] = 'X';
  au8Data[i++] = 'X';
  au8Data[i++] = 'F';
  au8Data[i++] = 'S';
  
  au8Data[i++] = ',';

  //�ճ�UINT16���ݳ��ȣ�������.λ��6��7
  au8Data[i++] = 0;    
  i++;    
  
  au8Data[i++] = ',';
  
  //�豸���
  au8Data[i++] = (UINT8)((sDisplay.u32TmlIndex & 0xff000000) >> 24);
  au8Data[i++] = (UINT8)((sDisplay.u32TmlIndex & 0x00ff0000) >> 16);
  au8Data[i++] = (UINT8)((sDisplay.u32TmlIndex & 0x0000ff00) >> 8);
  au8Data[i++] = (UINT8)(sDisplay.u32TmlIndex & 0x000000ff);
  
  au8Data[i++] = ',';

  //�����豸����
  au8Data[i++] = sDisplay.u8TmlType;

  au8Data[i++] = ',';

  //�����豸�ͺ�
  for(j=0; j<sDisplay.u8InfoLen; j++) 
  {
    au8Data[i++] = sDisplay.au8TmlInfo[j];
  }
  
  au8Data[i++] = ',';

  //���͵�γ����Ϣ
  for(j=0; j<9; j++)
  {
    au8Data[i++] = sGPSBuf[GPS_MAX_SIZE-1].GPS_X[j];
  }

  au8Data[i++] = ',';

  //�����ٶ���Ϣ
  for(j=0; j<10; j++)
  {
	au8Data[i++] = sGPSBuf[GPS_MAX_SIZE-1].GPS_Y[j];
  }

  au8Data[i++] = ',';
  
  //���;�����Ϣ
  for(j=0; j<5; j++)
  {
	  au8Data[i++] = sGPSBuf[GPS_MAX_SIZE-1].Course[j];
  }
  
  au8Data[i++] = ',';
  
  //���ͷ�λ��
  for(j=0; j<5; j++)
  {
	  au8Data[i++] = sGPSBuf[GPS_MAX_SIZE-1].Speed[j];
  }

  //���͵�����Ϣ
  au8Data[i++] = sSendData.u8Battery;
  
  au8Data[i++] = ',';

  //У��ǰ׺
  au8Data[i++] = '*';
  for(j=1; j<au8Data[4]+1; j++)
  {
    u8CheckSum ^= au8Data[j];
  }

  au8Data[i++] = u8CheckSum;

  //�������ݳ���
  au8Data[7] = i;

  au8Data[i++] = '\r';

  Delay(50);
  UartSendBuffer(UART1, &au8Data[0], i);
  Delay(3000);
}

/******************************************************************************
 * �������ƣ�DtuInfoSend
 * ������������Ϣ�����ġ�
 *
 * ��������
 * ���أ���
 *****************************************************************************/
void DtuInfoSend(void)
{
  if(g_bTmlStart == TRUE)
  {
    if(sDisplay.bDtuValid == TRUE)
    {
      if(g_bDtuFlag == TRUE)
      {
        if(g_bDtuSend == TRUE)
        {
          g_bDtuFlag = FALSE;
          g_bDtuSend = FALSE;
    
          SendTargetInfo();
    
          MEM_SET(&sSendData.u8Index, 0, sizeof(sSendData));
    
          g_bDtuSendCompleted = TRUE;
          
    #ifdef DEBUG
        UartSendString(UART2, "��Ϣ������ɣ�\r\n");
    #endif
        }
      }
    }
  }
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                   DTU�����豸��Ϣ����
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DtuCmdInit(void)
{
  sUartCmdCb[DTU_TYPE].eAssembleState = E_UART_CMD_RECEIVING_HEADER;
  sUartCmdCb[DTU_TYPE].pu8AssembleBuf = (UINT8 *)au8DtuBodyBuf;
  sUartCmdCb[DTU_TYPE].u8BufDataLength = 0;

  //����0���ճ�ʱ��ʱ����
  asTimer[TIMER_UART1].u16LifeTime = TIMER_TICK / 2;
  asTimer[TIMER_UART1].u16LifeOldTime = TIMER_TICK / 2;
  asTimer[TIMER_UART1].bEnable = FALSE;
}


/***********************************************************************************************
  �������ƣ�UartCmdAssemble(UINT8 u8InData)
  �������ܣ��ѽ��յ�������ϳ���Ч�����ݰ�

  ���룺u8InData
  �������
***********************************************************************************************/
void Uart1CmdAssemble(UINT8 u8InData)
{
  switch(sUartCmdCb[DTU_TYPE].eAssembleState)
  {
    case E_UART_CMD_RECEIVING_HEADER:
    {
      if(u8InData == '$')
      {
        sUartCmdCb[DTU_TYPE].pu8AssembleBuf[0] = u8InData;
        sUartCmdCb[DTU_TYPE].u8BufDataLength = 1;
	sUartCmdCb[DTU_TYPE].eAssembleState = E_UART_CMD_RECEIVING_BODY;	
      }
      break;
     }	
		
    case E_UART_CMD_RECEIVING_BODY:
    {
      sUartCmdCb[DTU_TYPE].pu8AssembleBuf[sUartCmdCb[DTU_TYPE].u8BufDataLength++] = u8InData;
      if(u8InData == '\r')
      {
        sUartCmdCb[DTU_TYPE].eAssembleState = E_UART_CMD_RECEIVING_VALID;
      }
      else
      {
        if(sUartCmdCb[DTU_TYPE].u8BufDataLength == MAX_BODY_SIZE)	
        {
          sUartCmdCb[DTU_TYPE].eAssembleState = E_UART_CMD_RECEIVING_HEADER;
          sUartCmdCb[DTU_TYPE].u8BufDataLength = 0;
          break;
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
      sUartCmdCb[DTU_TYPE].eAssembleState = E_UART_CMD_RECEIVING_HEADER;
      sUartCmdCb[DTU_TYPE].u8BufDataLength = 0;
    }
  }	
}


/***************************************************************************
//�������ƣ�CmdSetRun
//����������������������
//
//��������
//���أ���
***************************************************************************/
void DtuCmdRun(void)
{
  UINT8 u8Cmd;
  UINT8 u8CheckSumState=CMD_NONE;

  while((Uart1GetRxDataLength() > 0) && (sUartCmdCb[DTU_TYPE].eAssembleState != E_UART_CMD_RECEIVING_VALID))
  {
    Uart1CmdAssemble(Uart1GetByte());

    if(asTimer[TIMER_UART1].bEnable == TRUE)
    {
      asTimer[TIMER_UART1].bEnable = FALSE;
    }
  }
  
  //���ݰ���Ч���Կ��õ�ָ����н���
    if(sUartCmdCb[DTU_TYPE].eAssembleState == E_UART_CMD_RECEIVING_VALID)
    {
      u8Cmd = ConfigPackType(au8DtuBodyBuf, sUartCmdCb[DTU_TYPE].u8BufDataLength);
      //����������
      CreatCmdIndex(au8DtuBodyBuf);  
      //��Ϣ��������
     switch(u8Cmd)
      {
        case XXPZ:
        {
          u8CheckSumState = ConfigInfoProcess(DTU_TYPE, au8DtuBodyBuf);   //��ȡ������Ϣ
          break;
        }
        
        case DMPZ:
        {
          u8CheckSumState = MainServerConfigProcess(DTU_TYPE, au8DtuBodyBuf);   //��ȡ������Ϣ
          break;
        }
        
        case S1PZ:
        {
          u8CheckSumState = Sub1ConfigProcess(DTU_TYPE, au8DtuBodyBuf);   //��ȡ������Ϣ
          break;
        }
        
        case S2PZ:
        {
          u8CheckSumState = Sub2ConfigProcess(DTU_TYPE, au8DtuBodyBuf);   //��ȡ������Ϣ
          break;
        }
        
        case S3PZ:
        {
          u8CheckSumState = Sub3ConfigProcess(DTU_TYPE, au8DtuBodyBuf);   //��ȡ������Ϣ
          break;
        }
        
        case S4PZ:
        {
          u8CheckSumState = Sub4ConfigProcess(DTU_TYPE, au8DtuBodyBuf);   //��ȡ������Ϣ
          break;
        }
         
        case ZDQD:
        {
          g_bTmlStart = TRUE;
          break;
        }
        
        case ZDTZ:
        {
          
          break;
        }
        
        case ZDGJ:
        {
          u8CheckSumState = PowerDownProcess(DTU_TYPE, au8DtuBodyBuf);          
          break;
        }
        
        case TSKG:
        {
          u8CheckSumState = DebugProcess(DTU_TYPE, au8DtuBodyBuf);
          break;
        }
        
        case BBHM:
        {
          u8CheckSumState = GetVersionProcess(DTU_TYPE, au8DtuBodyBuf);
          break;
        }
        
        case DTHM:
        {
          u8CheckSumState = ReadDtuConfigProcess(DTU_TYPE, au8DtuBodyBuf);
          break;
        }
        
        default: u8CheckSumState = CMD_NONE;
      }
      
     //��ӡ������Ϣ������״̬ 
     if(g_bDebug == TRUE)
     {
        switch(u8CheckSumState)
        {
          case CMD_NONE:  
          {
            UartSendString(UART2, "��Ч���������\r\n"); 
            break;
          }
          
          case CMD_XXPZ_OK:
          {
            UartSendString(UART2, "��ϢԶ�����óɹ���\r\n");
            break;
          }
          
          case CMD_XXPZ_ERROR:
          {
            UartSendString(UART2, "��ϢԶ������ʧ�ܣ�\r\n");
            break;
          }
          
          case CMD_DMPZ_OK:
          {
            UartSendString(UART2, "��������Զ�����óɹ���\r\n");
            break;
          }
          
          case CMD_DMPZ_ERROR:
          {
            UartSendString(UART2, "��������Զ������ʧ�ܣ�\r\n");
            break;
          }
          
          case CMD_S1PZ_OK:
          {
            UartSendString(UART2, "������1Զ�����óɹ���\r\n");
            break;
          }
          
          case CMD_S1PZ_ERROR:
          {
            UartSendString(UART2, "������1Զ������ʧ�ܣ�\r\n");
            break;
          }
          
          case CMD_S2PZ_OK:
          {
            UartSendString(UART2, "������2Զ�����óɹ���\r\n");
            break;
          }
          
          case CMD_S2PZ_ERROR:
          {
            UartSendString(UART2, "������2Զ������ʧ�ܣ�\r\n");
            break;
          }
          
          case CMD_S3PZ_OK:
          {
            UartSendString(UART2, "������3Զ�����óɹ���\r\n");
            break;
          }
          
          case CMD_S3PZ_ERROR:
          {
            UartSendString(UART2, "������3Զ������ʧ�ܣ�\r\n");
            break;
          }
          
          case CMD_S4PZ_OK:
          {
            UartSendString(UART2, "������4Զ�����óɹ���\r\n");
            break;
          }
          
          case CMD_S4PZ_ERROR:
          {
            UartSendString(UART2, "������4Զ������ʧ�ܣ�\r\n");
            break;
          }
          
          default: break;
        }
     }
      //���������ָ����buf
      MEM_SET(&au8DtuBodyBuf[0], 0, sizeof(au8DtuBodyBuf));
      sUartCmdCb[DTU_TYPE].eAssembleState = E_UART_CMD_RECEIVING_HEADER;
      sUartCmdCb[DTU_TYPE].u8BufDataLength = 0;
    }
    else
    {
      //�涨ʱ���������ݣ�ָ�λ��������ȡ��Ч����
      if(asTimer[TIMER_UART1].bEnable == TRUE)                
      {
        if(asTimer[TIMER_UART1].u16LifeTime == 0)
        {
          sUartCmdCb[DTU_TYPE].eAssembleState = E_UART_CMD_RECEIVING_HEADER;
          sUartCmdCb[DTU_TYPE].u8BufDataLength = 0;

          asTimer[TIMER_UART1].bEnable = FALSE;
        }	
       }
       else
       {
          if(sUartCmdCb[DTU_TYPE].u8BufDataLength != 0)	
          {
            asTimer[TIMER_UART1].u16LifeTime = asTimer[TIMER_UART1].u16LifeOldTime;
            asTimer[TIMER_UART1].bEnable = TRUE;
          }
        }	
     }

  sDisplay.bDtuValid = DtuSvrCheck();
}

