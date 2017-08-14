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
#ifdef MSP430F149
#include "msp430x14x.h"
#else
#include "msp430x54x.h"
#endif

#include "global.h"
#include "DataType.h"
#include "GpsParse.h"
#include "DtuModule.h"
#include "UartCmd.h"
#include "Timer.h"
#include "Display.h"
#include "SystemCtl.h"

_tsUartCmdCb sUsbCmdCb;

//DTU���ջ�����
UINT8 au8UsbBodyBuf[MAX_BODY_SIZE];

//ǰ���ǿ������ת��ȱʡ����ɼ������
#define _GetWordFromAddr(pAddr)         (UINT16)(((UINT16)pAddr[0]) << 8) + pAddr[1]

#define _GetDoubleWordFromAddr(pAddr)   ((UINT32)(((UINT32)pAddr[0]) << 24) +    \
                                         (UINT32)(((UINT32)pAddr[1]) << 16) +    \
                                         (UINT32)(((UINT32)pAddr[2]) << 8) + pAddr[3])


const UINT8 *pCfgCmd[] = {"$XXPZ", "$DMPZ", "$S1PZ", "$S2PZ", "$S3PZ", "$S4PZ", "$ZDQD", "$ZDTZ", "$ZDGJ", "$TSKG", "$BBHM", "$YJCX"};


/* ====================================================================================================== */
//                                  ��������ú�������
/* ====================================================================================================== */

/***********************************************************************************
  �������ƣ�CreatCmdIndex
  �������ܣ�Ѱ�Ҷ���λ�ã�����������������ԭ�еĶ����滻Ϊ0x00

  ���룺pu8Buf -- �����������ݵĻ�����
  �������
***********************************************************************************/
UINT8 au8CmdIndex[32];    //�������������
void CreatCmdIndex(UINT8 * pu8Buf)
{
  UINT8 i, len;
  UINT8 j=0;

  MEM_SET(au8CmdIndex, 0, sizeof(au8CmdIndex));

  len = MAX_BODY_SIZE;
  for(i=0; i<len; i++)
  {
     if(pu8Buf[i] == ',')
     {
        au8CmdIndex[j++] = i;
//        pu8Buf[i] = 0x00;
     }
  }
}

/***********************************************************************************
  �������ƣ�RealCmdProcess
  �������ܣ������������ݵ�N��������ƫ��

  ���룺����������Ľ���GPS���ݻ�����
  ��������ص�N��","֮�����Ϣ����Ҫ*buffer��Ч������������ſ���ִ��
***********************************************************************************/
UINT8* RealCmdProcess( UINT8* pu8Buf, UINT8 num )
{
  if ( num < 1 )
    return  &pu8Buf[0];
  return  &pu8Buf[ au8CmdIndex[num - 1] + 1];
}

/**********************************************************************************************
 �������ƣ�CalcCheckSum
 �������ܣ�����buf��У��ͣ�����*����

 ������ *pu8Buf -- bufָ��
         u8Len  -- ���ݳ���
 ���أ� У���״̬
**********************************************************************************************/
UINT8 CalcCheckSum(UINT8 *pu8Buf, UINT8 u8Len)
{
  UINT8 u8CheckSum = 0;
  UINT8 i;
  
  //���㷢��У���
  for(i=1; i<u8Len; i++)
  {
    if(pu8Buf[i] != '*')
    {
      u8CheckSum ^= pu8Buf[i];
    }
    else
    {
      break;
    }
  }
  
  return u8CheckSum;
}

/**********************************************************************************************
 �������ƣ�CalcDataLen
 �������ܣ�����buf�н�������֡�����ݳ���

 ������ *pu8Buf -- bufָ��
        u8Len  -- ���ݳ���
 ���أ� ���ݰ��ĳ���
**********************************************************************************************/
UINT16 CalcDataLen(UINT8 *pu8Buf, UINT16 u16Len)
{
  UINT16 u16DataLen = 0;
  UINT16 i;
  
  //���㷢��У���
  for(i=0; i<u16Len; i++)
  {
    if(pu8Buf[i] == '*')
    {
      u16DataLen = i + 2;
      break;
    }
  }
  
  return u16DataLen;
}


/**********************************************************************************************
 �������ƣ�ConfigInfoProcess
 �������ܣ����������������������Ϣcopy����Ϣ���ݽṹ���򿪱��档

 ������ u8Index   ��Ҫ������ͨ����USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 ���أ� У���״̬
**********************************************************************************************/
UINT8 ConfigInfoProcess(UINT8 u8Index, UINT8 *pu8RxBuffer)
{
  UINT8 i=0;
  UINT8 *pu8Buf;
  UINT8 *pu8Temp;
  UINT16 u16DataLen=0;
  UINT8 u8CheckSum=0;
  UINT8 au8SendBuf[MAX_BODY_SIZE];
  UINT8 u8Temp=0;   //�������״̬���������е��±�λ��
  
  if((u8Index != USB_TYPE) && (u8Index != DTU_TYPE))
  {
    return TYPE_ERROR;
  }
  
  //����У���
  for(i=1; i<MAX_BODY_SIZE; i++)
  {
    if(pu8RxBuffer[i] != '*')
    {
      u8CheckSum ^= pu8RxBuffer[i];
    }
    else
    {
      //�����ݳ��ȸ���Ϊʵ�ʵ����ݳ���
      u16DataLen = i+2;
      break;
    }
  }
  
  MEM_SET(au8SendBuf, 0, MAX_BODY_SIZE);
  MEM_CPY(au8SendBuf, pu8RxBuffer, u16DataLen+1);

  au8SendBuf[1] = 'P';
  au8SendBuf[2] = 'Z';
  au8SendBuf[3] = 'F';
  au8SendBuf[4] = 'K';
  //����������
//  CreatCmdIndex(pu8RxBuffer);          
  
  //����ʵ�����ݳ��ȴ���buffer
  pu8Buf = RealCmdProcess(au8SendBuf, XXPZ_DATA_LEN);
  pu8Buf++;
  u8Temp = pu8Buf - au8SendBuf;
  au8SendBuf[u8Temp] = u16DataLen;
  
  //��ȡУ���
  pu8Temp = RealCmdProcess(au8SendBuf, XXPZ_CHECK_SUM);
  pu8Temp++;
  if(u8CheckSum != *pu8Temp)
  {  
    //����״̬��ֵ
    pu8Buf = RealCmdProcess(au8SendBuf, XXPZ_RESERVED);
    u8Temp = pu8Buf - au8SendBuf;
    au8SendBuf[u8Temp] = CFG_ERROR;
    
    u8CheckSum = 0;
    //���㷢��У���
    for(i=1; i<u16DataLen; i++)
    {
      if(au8SendBuf[i] != '*')
      {
        u8CheckSum ^= au8SendBuf[i];
      }
      else
      {
        break;
      }
    }
    u8Temp = pu8Temp - au8SendBuf;
    au8SendBuf[u8Temp] = u8CheckSum;
    
    if(u8Index == USB_TYPE)
    {
      UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
      }
    }
    
    return CMD_XXPZ_ERROR;
  }
  else
  {
    
    //����״̬��ֵ
    pu8Buf = RealCmdProcess(au8SendBuf, XXPZ_RESERVED);
    u8Temp = pu8Buf - au8SendBuf;
    au8SendBuf[u8Temp] = DTU_CFG_OK;
      
    u8CheckSum = 0;
    //���㷢��У���
    for(i=1; i<u16DataLen; i++)
    {
      if(au8SendBuf[i] != '*')
      {
        u8CheckSum ^= au8SendBuf[i];
      }
      else
      {
        break;
      }
    }
    u8Temp = pu8Temp - au8SendBuf;
    au8SendBuf[u8Temp] = u8CheckSum;
  
    if(u8Index == USB_TYPE)
    {
      UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
      }
    }
  ////////////////////////////////////////////////////////////////////////////////////////////////// 
    //                           �������㣬�ȽϺ�ʱ
    //��ȡ�豸���
    pu8Buf = RealCmdProcess(pu8RxBuffer, XXPZ_TML_INDEX);
    sDisplay.u32TmlIndex = _GetDoubleWordFromAddr(pu8Buf);
    sDisplay.u32TmlIndex = sDisplay.u32TmlIndex % 1000000;
    //�豸��Ż���Ϊ6λ��ʾ����
    sDisplay.au8TmlIndex[0] = (UINT8)(sDisplay.u32TmlIndex/100000 + 0x30);
    sDisplay.au8TmlIndex[1] = (UINT8)(sDisplay.u32TmlIndex/10000%10 + 0x30);
    sDisplay.au8TmlIndex[2] = (UINT8)(sDisplay.u32TmlIndex/1000%10 + 0x30);
    sDisplay.au8TmlIndex[3] = (UINT8)(sDisplay.u32TmlIndex/100%10 + 0x30);
    sDisplay.au8TmlIndex[4] = (UINT8)(sDisplay.u32TmlIndex/10%10 + 0x30);
    sDisplay.au8TmlIndex[5] = (UINT8)(sDisplay.u32TmlIndex%10 + 0x30);
  //////////////////////////////////////////////////////////////////////////////////////////////////   
    MEM_SET(&sDisplay.au8TmlInfo[0], ' ', 32);
/*    
    //ȥ���öδ��룬ֱ����PC���ͺ��ֱ���
    //��ȡ�豸����
    sDisplay.u8TmlType = *RealCmdProcess(pu8RxBuffer, XXPZ_TML_TYPE);
    if(sDisplay.u8TmlType == E_TML_TYPE_MACHINE) 
    {
      MEM_CPY(&sDisplay.au8TmlInfo[0], "���ͻ�", 6);
    }
    else 
    {
      if(sDisplay.u8TmlType == E_TML_TYPE_RAIL) 
      {
        MEM_CPY(&sDisplay.au8TmlInfo[0], "Χ����", 6);
      }
    }
*/   
    //��ȡ�豸�ͺ�
    pu8Buf = RealCmdProcess(pu8RxBuffer, XXPZ_TML_INFO);
    pu8Temp = RealCmdProcess(pu8RxBuffer, XXPZ_SEND_TIME);
    //��ȡ�豸�ͺ���Ϣ�ĳ���
    sDisplay.u8InfoLen = pu8Temp - pu8Buf - 1;  //��������֮�����豸�ͺŵĳ��ȣ���Ҫ��ȥ1�����š�
    for(i=0; i<sDisplay.u8InfoLen; i++)
    {
       sDisplay.au8TmlInfo[i] = *pu8Buf;
       pu8Buf++;
    }
    
    //��ȡʱ����
    pu8Buf = RealCmdProcess(pu8RxBuffer, XXPZ_SEND_TIME);
    if((*pu8Buf > 0) && (*pu8Buf < 7))
    {
      sDisplay.u8TimeSpace = *pu8Buf;
    }
    else
    {
      if(*pu8Buf < 1)
      {
        sDisplay.u8TimeSpace = E_TIME_MINUTE_5;
      }
      else if(*pu8Buf > 6)
      {
        sDisplay.u8TimeSpace = E_TIME_MINUTE_30;
      }
    }
    sDisplay.u8TimeWake = sDisplay.u8TimeSpace * 5;//ת��Ϊʵ�ʵķ���������
//    sDisplay.u32TimeSpace = Index2Second(sDisplay.u8TimeSpace);
    
    //�򿪱���ʹ��
    g_u8NeedSaved |= SAVE_TML_INFO;
    //����ʾˢ��
    sDisplay.bRefresh = TRUE;
    
    return CMD_XXPZ_OK;
  }
}

//�˺���Ϊ��ȡ�ַ�������
UINT8 CalcStringLen(UINT8 *s)
{
  UINT8 i=0;
  
  while(s[i] != 0)
  {
    i++;
  }
  
  if(i>0)
    return i;
  else
    return 0;
}

/**********************************************************************************************
 �������ƣ�MainServerConfigProcess
 �������ܣ�DTU���������������

 ������ u8Index   ��Ҫ������ͨ����USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 ���أ� У���״̬
**********************************************************************************************/
UINT8 MainServerConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer)
{
  UINT8 i;
  UINT8 *pu8Buf, *pu8Temp;
  UINT8 u8Temp;
  UINT16 u16DataLen=0;
  UINT8 u8CheckSum=0;
  UINT8 au8SendBuf[MAX_BODY_SIZE];
  UINT8 u8State = 0xff;
  BOOL bSaved = FALSE;
  
  if((u8Index != USB_TYPE) && (u8Index != DTU_TYPE))
  {
    return TYPE_ERROR;
  }
  
  //�����������ݰ���У��ͣ����ݳ���
  u8CheckSum = CalcCheckSum(pu8RxBuffer, MAX_BODY_SIZE);
  u16DataLen = CalcDataLen(pu8RxBuffer, MAX_BODY_SIZE);
    
  MEM_SET(au8SendBuf, 0, MAX_BODY_SIZE);
  MEM_CPY(au8SendBuf, pu8RxBuffer, u16DataLen + 1);
  
  //����BUf�����ʼ��
  au8SendBuf[1] = 'D';
  au8SendBuf[2] = 'M';
  au8SendBuf[3] = 'F';
  au8SendBuf[4] = 'K';
  
  pu8Temp = RealCmdProcess(au8SendBuf, DMPZ_CFG_CHK);
  pu8Temp++;
  //�ȶ�У����Ƿ���ȷ
  if(u8CheckSum != *pu8Temp)
  {
    //����״̬��ֵ
    pu8Buf = RealCmdProcess(au8SendBuf, DMPZ_CFG_RESERVED);
    u8Temp = pu8Buf - au8SendBuf;
    au8SendBuf[u8Temp] = DTU_CFG_ERROR;
    
    //���㷢��У���
    u8CheckSum = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
    u8Temp = pu8Temp - au8SendBuf;
    au8SendBuf[u8Temp] = u8CheckSum;
    
    if(u8Index == USB_TYPE)
    {
      UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
      }
    }
    
    return CMD_DMPZ_ERROR;
  }
  else
  {
    //��ȡ����ʹ��
    pu8Buf = RealCmdProcess(au8SendBuf, DMPZ_CFG_EN);
    if(*pu8Buf == DTU_CFG_VIEW)    //DTU����Ҫ���ã����Ĳ鿴DTU����Ϣ
    {
      //��ȡ����Э��
      pu8Buf = RealCmdProcess(au8SendBuf, DMPZ_CFG_TU);
      u8Temp = pu8Buf - au8SendBuf;
      au8SendBuf[u8Temp++] = sMainServer.bBckTcpUdp;
      au8SendBuf[u8Temp++] = ',';
      //��ȡ����APN��
      au8SendBuf[u8Temp++] = sMainServer.bBckApn;
      au8SendBuf[u8Temp++] = ',';
      //��ȡ�����û���
      i = CalcStringLen(&sMainServer.au8BckUserName[0]);
      if(i>16) i = 16;
      MEM_CPY(&au8SendBuf[u8Temp], &sMainServer.au8BckUserName[0], i);
      u8Temp += i;
      au8SendBuf[u8Temp++] = ',';
      //��ȡ��������
      i = CalcStringLen(&sMainServer.au8BckUserName[0]);
      if(i>16) i = 16;
      MEM_CPY(&au8SendBuf[u8Temp], &sMainServer.au8BckPassword[0], i);
      u8Temp += i;
      au8SendBuf[u8Temp++] = ',';
      //��ȡIP��ַ
      i = CalcStringLen(&sMainServer.au8BckIpAddr[0]);
      if(i>64) i = 64;
      MEM_CPY(&au8SendBuf[u8Temp], &sMainServer.au8BckIpAddr[0], i);
      u8Temp += i;
      au8SendBuf[u8Temp++] = ',';
      //��ȡ�˿ں�
      au8SendBuf[u8Temp++] = sMainServer.au8BckComIndex[0];
      au8SendBuf[u8Temp++] = sMainServer.au8BckComIndex[1];
      au8SendBuf[u8Temp++] = ',';
      //����״̬
      au8SendBuf[u8Temp++] = DTU_CFG_OK;
      au8SendBuf[u8Temp++] = ',';
      
      au8SendBuf[u8Temp++] = '*';
      au8SendBuf[u8Temp++] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
      au8SendBuf[u8Temp]   = '\r';
      
      //��ȡ���ݳ���λ��
      pu8Buf = RealCmdProcess(au8SendBuf, DMPZ_CFG_LEN);
      pu8Buf++;
      i = pu8Buf - au8SendBuf;
      au8SendBuf[i]   = u8Temp;
      u16DataLen = u8Temp;
    }
    else
    {
      if(*pu8Buf == DTU_CFG_ENABLE)
      {
        pu8Buf = RealCmdProcess(au8SendBuf, DMPZ_CFG_RESERVED);
        u8Temp = pu8Buf - au8SendBuf;
        au8SendBuf[u8Temp] = DTU_CFG_OK;
        
        u8Temp = pu8Temp - au8SendBuf;
        au8SendBuf[u8Temp] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
        
        //��ȡ���ݳ���λ��
        pu8Buf = RealCmdProcess(au8SendBuf, DMPZ_CFG_LEN);
        pu8Buf++;
        i = pu8Buf - au8SendBuf;
        au8SendBuf[i]  = CalcDataLen(au8SendBuf, MAX_BODY_SIZE);
        u16DataLen = au8SendBuf[i];
        
        bSaved = TRUE;
    
      }
    }
    //���ͷ�����Ϣ
    if(u8Index == USB_TYPE)
    {
       UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
      }
    }
    g_u8TimerDelay = 0;
    while(g_u8TimerDelay < 10);
    
    //����DTU������������
    if(bSaved == TRUE)
    {
      //��������������û�����
      MEM_SET(&sMainServer.bCfgTcpUdp, 0, (sizeof(_tsMainServer)/2));
//=================================================================================================================================
//    ����      
      //��ȡ����ID��ΪDTU��ID
      pu8Buf = RealCmdProcess(pu8RxBuffer, DMPZ_CFG_ID);
      pu8Temp = RealCmdProcess(pu8RxBuffer, DMPZ_CFG_EN);
      u8Temp = pu8Temp - pu8Buf - 1;  //��ȡDTU��ʾ������
      MEM_CPY(&sMainServer.au8CfgId[0], pu8Buf, u8Temp);
      
//=================================================================================================================================
     //��ȡTCP��UDP����
      pu8Buf = RealCmdProcess(pu8RxBuffer, DMPZ_CFG_TU);
      sMainServer.bCfgTcpUdp = *pu8Buf;

      //��ȡAPN��
      pu8Buf = RealCmdProcess(pu8RxBuffer, DMPZ_CFG_APN);
      sMainServer.bCfgApn = *pu8Buf;

      //��ȡ�����û���
      pu8Buf = RealCmdProcess(pu8RxBuffer, DMPZ_CFG_NAME);
      pu8Temp = RealCmdProcess(pu8RxBuffer, DMPZ_CFG_PWD);
      sMainServer.u8CfgUserNameLen = pu8Temp - pu8Buf - 1;  //��ȥһ���ָ����ռ�
      MEM_CPY(&sMainServer.au8CfgUserName[0], pu8Buf, sMainServer.u8CfgUserNameLen);
      
      //��ȡ�����û�����
      pu8Buf = RealCmdProcess(pu8RxBuffer, DMPZ_CFG_PWD);
      pu8Temp = RealCmdProcess(pu8RxBuffer, DMPZ_CFG_IP);
      sMainServer.u8CfgPasswordLen = pu8Temp - pu8Buf -1;  //��ȥһ���ָ����ռ�
      MEM_CPY(&sMainServer.au8CfgPassword[0], pu8Buf, sMainServer.u8CfgPasswordLen);
      
      //���IP��ַ����
      pu8Buf = RealCmdProcess(pu8RxBuffer, DMPZ_CFG_IP);
      pu8Temp = RealCmdProcess(pu8RxBuffer, DMPZ_CFG_COM);
      sMainServer.u8CfgIpLen = pu8Temp - pu8Buf - 1;     //��ȥһ���ָ����ռ�
      MEM_CPY(&sMainServer.au8CfgIpAddr[0], pu8Buf, sMainServer.u8CfgIpLen);
      
      //������ö˿�
      sMainServer.au8CfgComIndex[0] = *pu8Temp++;
      sMainServer.au8CfgComIndex[1] = *pu8Temp;
      
      if(0 != MEM_CMP(&sMainServer.bCfgTcpUdp, &sMainServer.bBckTcpUdp, (sizeof(_tsMainServer)/2)))
      { 
        MEM_SET(&sMainServer.bBckTcpUdp, 0, (sizeof(_tsMainServer)/2));
        MEM_CPY(&sMainServer.bBckTcpUdp, &sMainServer.bCfgTcpUdp, (sizeof(_tsMainServer)/2));
        
        //�رմ���0��2
        UartDisableInterrupt(UART0);
        UartDisableInterrupt(UART2);
        
        u8State = McuConfigDtuMainServer();
        
        UartEnableInterrupt(UART0);
        UartEnableInterrupt(UART2);
        
        if(u8State == DTU_CFG_OK)
          return CMD_DMPZ_OK;
       else
       {
          pu8Buf = RealCmdProcess(au8SendBuf, DMPZ_CFG_RESERVED);
          u8Temp = pu8Buf - au8SendBuf;
          au8SendBuf[u8Temp] = u8State;   //����ʧ�ܣ��Ѿ���Ĵ���״̬���ٴη��͸����ġ�
          
          u8Temp = pu8Temp - au8SendBuf;
          au8SendBuf[u8Temp] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
          
          //��ȡ���ݳ���λ��
          pu8Buf = RealCmdProcess(au8SendBuf, DMPZ_CFG_LEN);
          pu8Buf++;
          i = pu8Buf - au8SendBuf;
          au8SendBuf[i]   = CalcStringLen(au8SendBuf);
          
          //���ͷ�����Ϣ
          if(u8Index == USB_TYPE)
          {
             UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
          }
          else
          {
            if(u8Index == DTU_TYPE)
            {
              UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
            }
          }
          
          return CMD_DMPZ_ERROR;
       }
      }
    }    
  } 
  return CMD_DMPZ_OK;
}

/**********************************************************************************************
 �������ƣ�Sub1ConfigProcess
 �������ܣ�DTU������1�������

 ������ u8Index   ��Ҫ������ͨ����USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 ���أ� У���״̬
**********************************************************************************************/
UINT8 Sub1ConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer)
{
  UINT8 i;
  UINT8 *pu8Buf, *pu8Temp;
  UINT8 u8Temp;
  UINT16 u16DataLen=0;
  UINT8 u8CheckSum=0;
  UINT8 au8SendBuf[MAX_BODY_SIZE];
  UINT8 u8State = 0xff;
  BOOL bSaved = FALSE;
  UINT8 u8CfgEnable;
  
  if((u8Index != USB_TYPE) && (u8Index != DTU_TYPE))
  {
    return TYPE_ERROR;
  }
  
  //�����������ݰ���У��ͣ����ݳ���
  u8CheckSum = CalcCheckSum(pu8RxBuffer, MAX_BODY_SIZE);
  u16DataLen = CalcDataLen(pu8RxBuffer, MAX_BODY_SIZE);
    
  MEM_SET(au8SendBuf, 0, MAX_BODY_SIZE);
  MEM_CPY(au8SendBuf, pu8RxBuffer, u16DataLen + 1);
  
  //����BUf�����ʼ��
  au8SendBuf[1] = 'S';
  au8SendBuf[2] = '1';
  au8SendBuf[3] = 'F';
  au8SendBuf[4] = 'K';
  
  pu8Temp = RealCmdProcess(au8SendBuf, S1PZ_CFG_CHK);
  pu8Temp++;
  //�ȶ�У����Ƿ���ȷ
  if(u8CheckSum != *pu8Temp)
  {
    //����״̬��ֵ
    pu8Buf = RealCmdProcess(au8SendBuf, S1PZ_CFG_RESERVED);
    u8Temp = pu8Buf - au8SendBuf;
    au8SendBuf[u8Temp] = DTU_CFG_ERROR;
    
    //���㷢��У���
    u8CheckSum = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
    u8Temp = pu8Temp - au8SendBuf;
    au8SendBuf[u8Temp] = u8CheckSum;
    
    if(u8Index == USB_TYPE)
    {
      UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
      }
    }
    
    return CMD_S1PZ_ERROR;
  }
  else
  {
    //��ȡ����ʹ��
    pu8Buf = RealCmdProcess(au8SendBuf, S1PZ_CFG_EN);
/*    if(*pu8Buf == DTU_CFG_VIEW)    //DTU����Ҫ���ã����Ĳ鿴DTU����Ϣ
    {
      pu8Buf = RealCmdProcess(au8SendBuf, S1PZ_CFG_IP);
      u8Temp = pu8Buf - au8SendBuf;
      
      //��ȡIP��ַ, �������������Ϊ61��byte
      i = CalcStringLen(&sDtuCfg[IP_SUB_1].au8BckIpAddr[0]);
      if(i>61) i = 61;
      MEM_CPY(&au8SendBuf[u8Temp], &sDtuCfg[IP_SUB_1].au8BckIpAddr[0], i);
      u8Temp += i;
      au8SendBuf[u8Temp++] = ',';
      
      //��ȡ�˿ں�
      au8SendBuf[u8Temp++] = sDtuCfg[IP_SUB_1].au8BckComIndex[0];
      au8SendBuf[u8Temp++] = sDtuCfg[IP_SUB_1].au8BckComIndex[1];
      au8SendBuf[u8Temp++] = ',';
      
      //����״̬
      au8SendBuf[u8Temp++] = DTU_CFG_OK;
      au8SendBuf[u8Temp++] = ',';
      
      au8SendBuf[u8Temp++] = '*';
      au8SendBuf[u8Temp++] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
      au8SendBuf[u8Temp]   = '\r';
      
      //��ȡ���ݳ���λ��
      pu8Buf = RealCmdProcess(au8SendBuf, S1PZ_CFG_LEN);
      pu8Buf++;
      i = pu8Buf - au8SendBuf;
      au8SendBuf[i]   = u8Temp;
      u16DataLen = u8Temp;
    }
    else
    {*/
      if((*pu8Buf == DTU_CFG_ENABLE) || (*pu8Buf == DTU_CFG_DISABLE))  //��Ҫ�Է�����1��������
      {
        
        if(*pu8Buf == DTU_CFG_ENABLE)
        {
          u8CfgEnable = DTU_CFG_ENABLE;
        }
        else
        {
          u8CfgEnable = DTU_CFG_DISABLE;
        }
        pu8Buf = RealCmdProcess(au8SendBuf, S1PZ_CFG_RESERVED);
        u8Temp = pu8Buf - au8SendBuf;
        au8SendBuf[u8Temp] = DTU_CFG_OK;
        
        u8Temp = pu8Temp - au8SendBuf;
        au8SendBuf[u8Temp] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
        
        //��ȡ���ݳ���λ��
        pu8Buf = RealCmdProcess(au8SendBuf, S1PZ_CFG_LEN);
        pu8Buf++;
        i = pu8Buf - au8SendBuf;
        au8SendBuf[i]  = CalcDataLen(au8SendBuf, MAX_BODY_SIZE);
        u16DataLen = au8SendBuf[i];
        
        bSaved = TRUE;
      }
//    }
    
    //���ͷ�����Ϣ
    if(u8Index == USB_TYPE)
    {
       UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
       g_u8TimerDelay = 0;
       while(g_u8TimerDelay < 10);
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
        g_u8TimerDelay = 0;
        while(g_u8TimerDelay < 10);
      }
    }
    
    if(bSaved == TRUE)
    {
      //��������������û�����
     MEM_SET(&sDtuCfg[IP_SUB_1].au8CfgIpAddr[0], 0, (sizeof(_tsDtuCfg)/2));
      
      pu8Buf = RealCmdProcess(pu8RxBuffer, S1PZ_CFG_EN);
      sDtuCfg[IP_SUB_1].bCfgEnable = *pu8Buf;
      u8CfgEnable = *pu8Buf;
      
      //���IP��ַ����
      pu8Buf = RealCmdProcess(pu8RxBuffer, S1PZ_CFG_IP);
      pu8Temp = RealCmdProcess(pu8RxBuffer, S1PZ_CFG_COM);
      sDtuCfg[IP_SUB_1].u8CfgIpLen = pu8Temp - pu8Buf - 1;     //��ȥһ���ָ����ռ�
      MEM_CPY(&sDtuCfg[IP_SUB_1].au8CfgIpAddr[0], pu8Buf, sDtuCfg[IP_SUB_1].u8CfgIpLen);
      
      //������ö˿�
      sDtuCfg[IP_SUB_1].au8CfgComIndex[0] = *pu8Temp++;
      sDtuCfg[IP_SUB_1].au8CfgComIndex[1] = *pu8Temp;
      
      if(0 != MEM_CMP(&sDtuCfg[IP_SUB_1].au8CfgIpAddr[0], &sDtuCfg[IP_SUB_1].au8BckIpAddr[0], (sizeof(_tsDtuCfg)/2)))
      { 
        MEM_SET(&sDtuCfg[IP_SUB_1].au8BckIpAddr[0], 0, (sizeof(_tsDtuCfg)/2));
        MEM_CPY(&sDtuCfg[IP_SUB_1].au8BckIpAddr[0], &sDtuCfg[IP_SUB_1].au8CfgIpAddr[0], (sizeof(_tsDtuCfg)/2));
        
        //�رմ���0��2
        UartDisableInterrupt(UART0);
        UartDisableInterrupt(UART2);
        
        u8State = McuConfigDtuSub1(u8CfgEnable);
        
        UartEnableInterrupt(UART0);
        UartEnableInterrupt(UART2);
        
        if(u8State == DTU_CFG_OK)
          return CMD_S1PZ_OK;
       else
       {
          pu8Buf = RealCmdProcess(au8SendBuf, S1PZ_CFG_RESERVED);
          u8Temp = pu8Buf - au8SendBuf;
          au8SendBuf[u8Temp] = u8State;   //����ʧ�ܣ��Ѿ���Ĵ���״̬���ٴη��͸����ġ�
          
          u8Temp = pu8Temp - au8SendBuf;
          au8SendBuf[u8Temp] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
          
          //��ȡ���ݳ���λ��
          pu8Buf = RealCmdProcess(au8SendBuf, S1PZ_CFG_LEN);
          pu8Buf++;
          i = pu8Buf - au8SendBuf;
          au8SendBuf[i]   = CalcStringLen(au8SendBuf);
          
          //���ͷ�����Ϣ
          if(u8Index == USB_TYPE)
          {
             UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
          }
          else
          {
            if(u8Index == DTU_TYPE)
            {
              UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
            }
          }
          
          return CMD_S1PZ_ERROR;
       }
      }
    }
  }
  
  return CMD_S1PZ_OK;
}

/**********************************************************************************************
 �������ƣ�Sub2ConfigProcess
 �������ܣ�DTU������1�������

 ������ u8Index   ��Ҫ������ͨ����USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 ���أ� У���״̬
**********************************************************************************************/
UINT8 Sub2ConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer)
{
    UINT8 i;
  UINT8 *pu8Buf, *pu8Temp;
  UINT8 u8Temp;
  UINT16 u16DataLen=0;
  UINT8 u8CheckSum=0;
  UINT8 au8SendBuf[MAX_BODY_SIZE];
  UINT8 u8State = 0xff;
  BOOL bSaved = FALSE;
  UINT8 u8CfgEnable;
  
  if((u8Index != USB_TYPE) && (u8Index != DTU_TYPE))
  {
    return TYPE_ERROR;
  }
  
  //�����������ݰ���У��ͣ����ݳ���
  u8CheckSum = CalcCheckSum(pu8RxBuffer, MAX_BODY_SIZE);
  u16DataLen = CalcDataLen(pu8RxBuffer, MAX_BODY_SIZE);
    
  MEM_SET(au8SendBuf, 0, MAX_BODY_SIZE);
  MEM_CPY(au8SendBuf, pu8RxBuffer, u16DataLen + 1);
  
  //����BUf�����ʼ��
  au8SendBuf[1] = 'S';
  au8SendBuf[2] = '2';
  au8SendBuf[3] = 'F';
  au8SendBuf[4] = 'K';
  
  pu8Temp = RealCmdProcess(au8SendBuf, S2PZ_CFG_CHK);
  pu8Temp++;
  //�ȶ�У����Ƿ���ȷ
  if(u8CheckSum != *pu8Temp)
  {
    //����״̬��ֵ
    pu8Buf = RealCmdProcess(au8SendBuf, S2PZ_CFG_RESERVED);
    u8Temp = pu8Buf - au8SendBuf;
    au8SendBuf[u8Temp] = DTU_CFG_ERROR;
    
    //���㷢��У���
    u8CheckSum = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
    u8Temp = pu8Temp - au8SendBuf;
    au8SendBuf[u8Temp] = u8CheckSum;
    
    if(u8Index == USB_TYPE)
    {
      UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
      }
    }
    
    return CMD_S2PZ_ERROR;
  }
  else
  {
    //��ȡ����ʹ��
    pu8Buf = RealCmdProcess(au8SendBuf, S2PZ_CFG_EN);
/*    if(*pu8Buf == DTU_CFG_VIEW)    //DTU����Ҫ���ã����Ĳ鿴DTU����Ϣ
    {
      pu8Buf = RealCmdProcess(au8SendBuf, S2PZ_CFG_IP);
      u8Temp = pu8Buf - au8SendBuf;
      
      //��ȡIP��ַ, �������������Ϊ61��byte
      i = CalcStringLen(&sDtuCfg[IP_SUB_2].au8BckIpAddr[0]);
      if(i>61) i = 61;
      MEM_CPY(&au8SendBuf[u8Temp], &sDtuCfg[IP_SUB_2].au8BckIpAddr[0], i);
      u8Temp += i;
      au8SendBuf[u8Temp++] = ',';
      
      //��ȡ�˿ں�
      au8SendBuf[u8Temp++] = sDtuCfg[IP_SUB_2].au8BckComIndex[0];
      au8SendBuf[u8Temp++] = sDtuCfg[IP_SUB_2].au8BckComIndex[1];
      au8SendBuf[u8Temp++] = ',';
      
      //����״̬
      au8SendBuf[u8Temp++] = DTU_CFG_OK;
      au8SendBuf[u8Temp++] = ',';
      
      au8SendBuf[u8Temp++] = '*';
      au8SendBuf[u8Temp++] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
      au8SendBuf[u8Temp]   = '\r';
      
      //��ȡ���ݳ���λ��
      pu8Buf = RealCmdProcess(au8SendBuf, S2PZ_CFG_LEN);
      pu8Buf++;
      i = pu8Buf - au8SendBuf;
      au8SendBuf[i]   = u8Temp;
      u16DataLen = u8Temp;
    }
    else
    {*/
      if((*pu8Buf == DTU_CFG_ENABLE) || (*pu8Buf == DTU_CFG_DISABLE))  //��Ҫ�Է�����1��������
      {
        
        if(*pu8Buf == DTU_CFG_ENABLE)
        {
          u8CfgEnable = DTU_CFG_ENABLE;
        }
        else
        {
          u8CfgEnable = DTU_CFG_DISABLE;
        }
        pu8Buf = RealCmdProcess(au8SendBuf, S2PZ_CFG_RESERVED);
        u8Temp = pu8Buf - au8SendBuf;
        au8SendBuf[u8Temp] = DTU_CFG_OK;
        
        u8Temp = pu8Temp - au8SendBuf;
        au8SendBuf[u8Temp] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
        
        //��ȡ���ݳ���λ��
        pu8Buf = RealCmdProcess(au8SendBuf, S2PZ_CFG_LEN);
        pu8Buf++;
        i = pu8Buf - au8SendBuf;
        au8SendBuf[i]  = CalcDataLen(au8SendBuf, MAX_BODY_SIZE);
        u16DataLen = au8SendBuf[i];
        
        bSaved = TRUE;
      }
//    }
    
    //���ͷ�����Ϣ
    if(u8Index == USB_TYPE)
    {
       UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
       g_u8TimerDelay = 0;
        while(g_u8TimerDelay < 10);
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
        g_u8TimerDelay = 0;
        while(g_u8TimerDelay < 10);
      }
    }
    
    if(bSaved == TRUE)
    {
      //��������������û�����
      MEM_SET(&sDtuCfg[IP_SUB_2].au8CfgIpAddr[0], 0, (sizeof(_tsDtuCfg)/2));
      
      pu8Buf = RealCmdProcess(pu8RxBuffer, S2PZ_CFG_EN);
      sDtuCfg[IP_SUB_2].bCfgEnable = *pu8Buf;
      u8CfgEnable = *pu8Buf;
      
      //���IP��ַ����
      pu8Buf = RealCmdProcess(pu8RxBuffer, S2PZ_CFG_IP);
      pu8Temp = RealCmdProcess(pu8RxBuffer, S2PZ_CFG_COM);
      sDtuCfg[IP_SUB_2].u8CfgIpLen = pu8Temp - pu8Buf - 1;     //��ȥһ���ָ����ռ�
      MEM_CPY(&sDtuCfg[IP_SUB_2].au8CfgIpAddr[0], pu8Buf, sDtuCfg[IP_SUB_2].u8CfgIpLen);
      
      //������ö˿�
      sDtuCfg[IP_SUB_2].au8CfgComIndex[0] = *pu8Temp++;
      sDtuCfg[IP_SUB_2].au8CfgComIndex[1] = *pu8Temp;
      
      if(0 != MEM_CMP(&sDtuCfg[IP_SUB_2].au8CfgIpAddr[0], &sDtuCfg[IP_SUB_2].au8BckIpAddr[0], (sizeof(_tsDtuCfg)/2)))
      { 
        MEM_SET(&sDtuCfg[IP_SUB_2].au8BckIpAddr[0], 0, (sizeof(_tsDtuCfg)/2));
        MEM_CPY(&sDtuCfg[IP_SUB_2].au8BckIpAddr[0], &sDtuCfg[IP_SUB_2].au8CfgIpAddr[0], (sizeof(_tsDtuCfg)/2));
        
        //�رմ���0��2
        UartDisableInterrupt(UART0);
        UartDisableInterrupt(UART2);
        
        u8State = McuConfigDtuSub2(u8CfgEnable);
        
        UartEnableInterrupt(UART0);
        UartEnableInterrupt(UART2);
        
        if(u8State == DTU_CFG_OK)
          return CMD_S2PZ_OK;
       else
       {
          pu8Buf = RealCmdProcess(au8SendBuf, S2PZ_CFG_RESERVED);
          u8Temp = pu8Buf - au8SendBuf;
          au8SendBuf[u8Temp] = u8State;   //����ʧ�ܣ��Ѿ���Ĵ���״̬���ٴη��͸����ġ�
          
          u8Temp = pu8Temp - au8SendBuf;
          au8SendBuf[u8Temp] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
          
          //��ȡ���ݳ���λ��
          pu8Buf = RealCmdProcess(au8SendBuf, S2PZ_CFG_LEN);
          pu8Buf++;
          i = pu8Buf - au8SendBuf;
          au8SendBuf[i]   = CalcStringLen(au8SendBuf);
          
          //���ͷ�����Ϣ
          if(u8Index == USB_TYPE)
          {
             UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
          }
          else
          {
            if(u8Index == DTU_TYPE)
            {
              UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
            }
          }
          
          return CMD_S2PZ_ERROR;
       }
      }
    }
  }
  
  return CMD_S2PZ_OK;
}

/**********************************************************************************************
 �������ƣ�Sub3ConfigProcess
 �������ܣ�DTU������1�������

 ������ u8Index   ��Ҫ������ͨ����USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 ���أ� У���״̬
**********************************************************************************************/
UINT8 Sub3ConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer)
{
  UINT8 i;
  UINT8 *pu8Buf, *pu8Temp;
  UINT8 u8Temp;
  UINT16 u16DataLen=0;
  UINT8 u8CheckSum=0;
  UINT8 au8SendBuf[MAX_BODY_SIZE];
  UINT8 u8State = 0xff;
  BOOL bSaved = FALSE;
  UINT8 u8CfgEnable;
  
  if((u8Index != USB_TYPE) && (u8Index != DTU_TYPE))
  {
    return TYPE_ERROR;
  }
  
  //�����������ݰ���У��ͣ����ݳ���
  u8CheckSum = CalcCheckSum(pu8RxBuffer, MAX_BODY_SIZE);
  u16DataLen = CalcDataLen(pu8RxBuffer, MAX_BODY_SIZE);
    
  MEM_SET(au8SendBuf, 0, MAX_BODY_SIZE);
  MEM_CPY(au8SendBuf, pu8RxBuffer, u16DataLen + 1);
  
  //����BUf�����ʼ��
  au8SendBuf[1] = 'S';
  au8SendBuf[2] = '3';
  au8SendBuf[3] = 'F';
  au8SendBuf[4] = 'K';
  
  pu8Temp = RealCmdProcess(au8SendBuf, S3PZ_CFG_CHK);
  pu8Temp++;
  //�ȶ�У����Ƿ���ȷ
  if(u8CheckSum != *pu8Temp)
  {
    //����״̬��ֵ
    pu8Buf = RealCmdProcess(au8SendBuf, S3PZ_CFG_RESERVED);
    u8Temp = pu8Buf - au8SendBuf;
    au8SendBuf[u8Temp] = DTU_CFG_ERROR;
    
    //���㷢��У���
    u8CheckSum = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
    u8Temp = pu8Temp - au8SendBuf;
    au8SendBuf[u8Temp] = u8CheckSum;
    
    if(u8Index == USB_TYPE)
    {
      UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
      }
    }
    
    return CMD_S3PZ_ERROR;
  }
  else
  {
    //��ȡ����ʹ��
    pu8Buf = RealCmdProcess(au8SendBuf, S3PZ_CFG_EN);
/*    if(*pu8Buf == DTU_CFG_VIEW)    //DTU����Ҫ���ã����Ĳ鿴DTU����Ϣ
    {
      pu8Buf = RealCmdProcess(au8SendBuf, S3PZ_CFG_IP);
      u8Temp = pu8Buf - au8SendBuf;
      
      //��ȡIP��ַ, �������������Ϊ61��byte
      i = CalcStringLen(&sDtuCfg[IP_SUB_3].au8BckIpAddr[0]);
      if(i>61) i = 61;
      MEM_CPY(&au8SendBuf[u8Temp], &sDtuCfg[IP_SUB_3].au8BckIpAddr[0], i);
      u8Temp += i;
      au8SendBuf[u8Temp++] = ',';
      
      //��ȡ�˿ں�
      au8SendBuf[u8Temp++] = sDtuCfg[IP_SUB_3].au8BckComIndex[0];
      au8SendBuf[u8Temp++] = sDtuCfg[IP_SUB_3].au8BckComIndex[1];
      au8SendBuf[u8Temp++] = ',';
      
      //����״̬
      au8SendBuf[u8Temp++] = DTU_CFG_OK;
      au8SendBuf[u8Temp++] = ',';
      
      au8SendBuf[u8Temp++] = '*';
      au8SendBuf[u8Temp++] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
      au8SendBuf[u8Temp]   = '\r';
      
      //��ȡ���ݳ���λ��
      pu8Buf = RealCmdProcess(au8SendBuf, S3PZ_CFG_LEN);
      pu8Buf++;
      i = pu8Buf - au8SendBuf;
      au8SendBuf[i]   = u8Temp;
      u16DataLen = u8Temp;
    }
    else
   {*/
      if((*pu8Buf == DTU_CFG_ENABLE) || (*pu8Buf == DTU_CFG_DISABLE))  //��Ҫ�Է�����1��������
      {
        
        if(*pu8Buf == DTU_CFG_ENABLE)
        {
          u8CfgEnable = DTU_CFG_ENABLE;
        }
        else
        {
          u8CfgEnable = DTU_CFG_DISABLE;
        }
        
        pu8Buf = RealCmdProcess(au8SendBuf, S3PZ_CFG_RESERVED);
        u8Temp = pu8Buf - au8SendBuf;
        au8SendBuf[u8Temp] = DTU_CFG_OK;
        
        u8Temp = pu8Temp - au8SendBuf;
        au8SendBuf[u8Temp] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
        
        //��ȡ���ݳ���λ��
        pu8Buf = RealCmdProcess(au8SendBuf, S3PZ_CFG_LEN);
        pu8Buf++;
        i = pu8Buf - au8SendBuf;
        au8SendBuf[i]  = CalcDataLen(au8SendBuf, MAX_BODY_SIZE);
        u16DataLen = au8SendBuf[i];
        
        bSaved = TRUE;
      }
 //   }
    
    //���ͷ�����Ϣ
    if(u8Index == USB_TYPE)
    {
       UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
       g_u8TimerDelay = 0;
        while(g_u8TimerDelay < 10);
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
        g_u8TimerDelay = 0;
        while(g_u8TimerDelay < 10);
      }
    }
    
    if(bSaved == TRUE)
    {
      //��������������û�����
      MEM_SET(&sDtuCfg[IP_SUB_3].au8CfgIpAddr[0], 0, (sizeof(_tsDtuCfg)/2));
      
      pu8Buf = RealCmdProcess(pu8RxBuffer, S3PZ_CFG_EN);
      sDtuCfg[IP_SUB_3].bCfgEnable = *pu8Buf;
      u8CfgEnable = *pu8Buf;
      
      //���IP��ַ����
      pu8Buf = RealCmdProcess(pu8RxBuffer, S3PZ_CFG_IP);
      pu8Temp = RealCmdProcess(pu8RxBuffer, S3PZ_CFG_COM);
      sDtuCfg[IP_SUB_3].u8CfgIpLen = pu8Temp - pu8Buf - 1;     //��ȥһ���ָ����ռ�
      MEM_CPY(&sDtuCfg[IP_SUB_3].au8CfgIpAddr[0], pu8Buf, sDtuCfg[IP_SUB_3].u8CfgIpLen);
      
      //������ö˿�
      sDtuCfg[IP_SUB_3].au8CfgComIndex[0] = *pu8Temp++;
      sDtuCfg[IP_SUB_3].au8CfgComIndex[1] = *pu8Temp;
      
      if(0 != MEM_CMP(&sDtuCfg[IP_SUB_1].au8CfgIpAddr[0], &sDtuCfg[IP_SUB_3].au8BckIpAddr[0], (sizeof(_tsDtuCfg)/2)))
      { 
        MEM_SET(&sDtuCfg[IP_SUB_3].au8BckIpAddr[0], 0, (sizeof(_tsDtuCfg)/2));
        MEM_CPY(&sDtuCfg[IP_SUB_3].au8BckIpAddr[0], &sDtuCfg[IP_SUB_3].au8CfgIpAddr[0], (sizeof(_tsDtuCfg)/2));
        
        //�رմ���0��2
        UartDisableInterrupt(UART0);
        UartDisableInterrupt(UART2);
        
        u8State = McuConfigDtuSub3(u8CfgEnable);
        
        UartEnableInterrupt(UART0);
        UartEnableInterrupt(UART2);
        
        if(u8State == DTU_CFG_OK)
          return CMD_S3PZ_OK;
       else
       {
          pu8Buf = RealCmdProcess(au8SendBuf, S3PZ_CFG_RESERVED);
          u8Temp = pu8Buf - au8SendBuf;
          au8SendBuf[u8Temp] = u8State;   //����ʧ�ܣ��Ѿ���Ĵ���״̬���ٴη��͸����ġ�
          
          u8Temp = pu8Temp - au8SendBuf;
          au8SendBuf[u8Temp] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
          
          //��ȡ���ݳ���λ��
          pu8Buf = RealCmdProcess(au8SendBuf, S3PZ_CFG_LEN);
          pu8Buf++;
          i = pu8Buf - au8SendBuf;
          au8SendBuf[i]   = CalcStringLen(au8SendBuf);
          
          //���ͷ�����Ϣ
          if(u8Index == USB_TYPE)
          {
             UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
          }
          else
          {
            if(u8Index == DTU_TYPE)
            {
              UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
            }
          }
          
          return CMD_S3PZ_ERROR;
       }
      }
    }
  }
  
  return CMD_S3PZ_OK;
}

/**********************************************************************************************
 �������ƣ�Sub4ConfigProcess
 �������ܣ�DTU������1�������

 ������ u8Index   ��Ҫ������ͨ����USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 ���أ� У���״̬
**********************************************************************************************/
UINT8 Sub4ConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer)
{
  UINT8 i;
  UINT8 *pu8Buf, *pu8Temp;
  UINT8 u8Temp;
  UINT16 u16DataLen=0;
  UINT8 u8CheckSum=0;
  UINT8 au8SendBuf[MAX_BODY_SIZE];
  UINT8 u8State = 0xff;
  BOOL bSaved = FALSE;
  UINT8 u8CfgEnable;
  
  if((u8Index != USB_TYPE) && (u8Index != DTU_TYPE))
  {
    return TYPE_ERROR;
  }
  
  //�����������ݰ���У��ͣ����ݳ���
  u8CheckSum = CalcCheckSum(pu8RxBuffer, MAX_BODY_SIZE);
  u16DataLen = CalcDataLen(pu8RxBuffer, MAX_BODY_SIZE);
    
  MEM_SET(au8SendBuf, 0, MAX_BODY_SIZE);
  MEM_CPY(au8SendBuf, pu8RxBuffer, u16DataLen + 1);
  
  //����BUf�����ʼ��
  au8SendBuf[1] = 'S';
  au8SendBuf[2] = '4';
  au8SendBuf[3] = 'F';
  au8SendBuf[4] = 'K';
  
  pu8Temp = RealCmdProcess(au8SendBuf, S4PZ_CFG_CHK);
  pu8Temp++;
  //�ȶ�У����Ƿ���ȷ
  if(u8CheckSum != *pu8Temp)
  {
    //����״̬��ֵ
    pu8Buf = RealCmdProcess(au8SendBuf, S4PZ_CFG_RESERVED);
    u8Temp = pu8Buf - au8SendBuf;
    au8SendBuf[u8Temp] = DTU_CFG_ERROR;
    
    //���㷢��У���
    u8CheckSum = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
    u8Temp = pu8Temp - au8SendBuf;
    au8SendBuf[u8Temp] = u8CheckSum;
    
    if(u8Index == USB_TYPE)
    {
      UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
      }
    }
    
    return CMD_S4PZ_ERROR;
  }
  else
  {
    //��ȡ����ʹ��
    pu8Buf = RealCmdProcess(au8SendBuf, S4PZ_CFG_EN);
/*    if(*pu8Buf == DTU_CFG_VIEW)    //DTU����Ҫ���ã����Ĳ鿴DTU����Ϣ
    {
      pu8Buf = RealCmdProcess(au8SendBuf, S4PZ_CFG_IP);
      u8Temp = pu8Buf - au8SendBuf;
      
      //��ȡIP��ַ, �������������Ϊ61��byte
      i = CalcStringLen(&sDtuCfg[IP_SUB_4].au8BckIpAddr[0]);
      if(i>61) i = 61;
      MEM_CPY(&au8SendBuf[u8Temp], &sDtuCfg[IP_SUB_4].au8BckIpAddr[0], i);
      u8Temp += i;
      au8SendBuf[u8Temp++] = ',';
      
      //��ȡ�˿ں�
      au8SendBuf[u8Temp++] = sDtuCfg[IP_SUB_4].au8BckComIndex[0];
      au8SendBuf[u8Temp++] = sDtuCfg[IP_SUB_4].au8BckComIndex[1];
      au8SendBuf[u8Temp++] = ',';
      
      //����״̬
      au8SendBuf[u8Temp++] = DTU_CFG_OK;
      au8SendBuf[u8Temp++] = ',';
      
      au8SendBuf[u8Temp++] = '*';
      au8SendBuf[u8Temp++] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
      au8SendBuf[u8Temp]   = '\r';
      
      //��ȡ���ݳ���λ��
      pu8Buf = RealCmdProcess(au8SendBuf, S4PZ_CFG_LEN);
      pu8Buf++;
      i = pu8Buf - au8SendBuf;
      au8SendBuf[i]   = u8Temp;
      u16DataLen = u8Temp;
    }
    else
    {*/
      if((*pu8Buf == DTU_CFG_ENABLE) || (*pu8Buf == DTU_CFG_DISABLE))  //��Ҫ�Է�����1��������
      {
        
        if(*pu8Buf == DTU_CFG_ENABLE)
        {
          u8CfgEnable = DTU_CFG_ENABLE;
        }
        else
        {
          u8CfgEnable = DTU_CFG_DISABLE;
        }
        pu8Buf = RealCmdProcess(au8SendBuf, S4PZ_CFG_RESERVED);
        u8Temp = pu8Buf - au8SendBuf;
        au8SendBuf[u8Temp] = DTU_CFG_OK;
        
        u8Temp = pu8Temp - au8SendBuf;
        au8SendBuf[u8Temp] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
        
        //��ȡ���ݳ���λ��
        pu8Buf = RealCmdProcess(au8SendBuf, S4PZ_CFG_LEN);
        pu8Buf++;
        i = pu8Buf - au8SendBuf;
        au8SendBuf[i]  = CalcDataLen(au8SendBuf, MAX_BODY_SIZE);
        u16DataLen = au8SendBuf[i];
        
        bSaved = TRUE;
      }
//    }
    
    //���ͷ�����Ϣ
    if(u8Index == USB_TYPE)
    {
       UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
       g_u8TimerDelay = 0;
        while(g_u8TimerDelay < 10);
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
        g_u8TimerDelay = 0;
        while(g_u8TimerDelay < 10);
      }
    }
    
    if(bSaved == TRUE)
    {
      //��������������û�����
      MEM_SET(&sDtuCfg[IP_SUB_4].au8CfgIpAddr[0], 0, (sizeof(_tsDtuCfg)/2));
      
      pu8Buf = RealCmdProcess(pu8RxBuffer, S4PZ_CFG_EN);
      sDtuCfg[IP_SUB_4].bCfgEnable = *pu8Buf;
      u8CfgEnable = *pu8Buf;
      
      //���IP��ַ����
      pu8Buf = RealCmdProcess(pu8RxBuffer, S4PZ_CFG_IP);
      pu8Temp = RealCmdProcess(pu8RxBuffer, S4PZ_CFG_COM);
      sDtuCfg[IP_SUB_4].u8CfgIpLen = pu8Temp - pu8Buf - 1;     //��ȥһ���ָ����ռ�
      MEM_CPY(&sDtuCfg[IP_SUB_4].au8CfgIpAddr[0], pu8Buf, sDtuCfg[IP_SUB_4].u8CfgIpLen);
      
      //������ö˿�
      sDtuCfg[IP_SUB_4].au8CfgComIndex[0] = *pu8Temp++;
      sDtuCfg[IP_SUB_4].au8CfgComIndex[1] = *pu8Temp;
      
      if(0 != MEM_CMP(&sDtuCfg[IP_SUB_4].au8CfgIpAddr[0], &sDtuCfg[IP_SUB_4].au8BckIpAddr[0], (sizeof(_tsDtuCfg)/2)))
      { 
        MEM_SET(&sDtuCfg[IP_SUB_4].au8BckIpAddr[0], 0, (sizeof(_tsDtuCfg)/2));
        MEM_CPY(&sDtuCfg[IP_SUB_4].au8BckIpAddr[0], &sDtuCfg[IP_SUB_4].au8CfgIpAddr[0], (sizeof(_tsDtuCfg)/2));
        
        //�رմ���0��2
        UartDisableInterrupt(UART0);
        UartDisableInterrupt(UART2);
        
        u8State = McuConfigDtuSub4(u8CfgEnable);
        
        UartEnableInterrupt(UART0);
        UartEnableInterrupt(UART2);
        
        if(u8State == DTU_CFG_OK)
          return CMD_S4PZ_OK;
       else
       {
          pu8Buf = RealCmdProcess(au8SendBuf, S4PZ_CFG_RESERVED);
          u8Temp = pu8Buf - au8SendBuf;
          au8SendBuf[u8Temp] = u8State;   //����ʧ�ܣ��Ѿ���Ĵ���״̬���ٴη��͸����ġ�
          
          u8Temp = pu8Temp - au8SendBuf;
          au8SendBuf[u8Temp] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
          
          //��ȡ���ݳ���λ��
          pu8Buf = RealCmdProcess(au8SendBuf, S4PZ_CFG_LEN);
          pu8Buf++;
          i = pu8Buf - au8SendBuf;
          au8SendBuf[i]   = CalcStringLen(au8SendBuf);
          
          //���ͷ�����Ϣ
          if(u8Index == USB_TYPE)
          {
             UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
          }
          else
          {
            if(u8Index == DTU_TYPE)
            {
              UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
            }
          }
          
          return CMD_S4PZ_ERROR;
       }
      }
    }
  }
  
  return CMD_S4PZ_OK;
}

/**********************************************************************************************
 �������ƣ� DebugProcess
 �������ܣ�debug�ӿڿ����رմ���

 ������ u8Index   ��Ҫ������ͨ����USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 ���أ� У���״̬
**********************************************************************************************/
UINT8 DebugProcess(UINT8 u8Index, UINT8 *pu8RxBuffer)
{
  UINT16 u16DataLen=0;
  UINT8 u8CheckSum=0;
  UINT8 au8SendBuf[20];

  if((u8Index != USB_TYPE) && (u8Index != DTU_TYPE))
  {
    return TYPE_ERROR;
  }
  
  //�����������ݰ���У��ͣ����ݳ���
  u8CheckSum = CalcCheckSum(pu8RxBuffer, MAX_BODY_SIZE);
  u16DataLen = CalcDataLen(pu8RxBuffer, MAX_BODY_SIZE);
    
  MEM_SET(au8SendBuf, 0, 20);
  MEM_CPY(au8SendBuf, pu8RxBuffer, u16DataLen + 1);
  
  //����BUf�����ʼ��
  au8SendBuf[1] = 'T';
  au8SendBuf[2] = 'S';
  au8SendBuf[3] = 'K';
  au8SendBuf[4] = 'G';
  
  //�ȶ�У����Ƿ���ȷ
  if(u8CheckSum != pu8RxBuffer[9])
  {
    //����״̬��ֵ
    au8SendBuf[6] = 0x03;
    
    //���㷢��У���
    au8SendBuf[9] = CalcCheckSum(au8SendBuf, 20);
    
    if(u8Index == USB_TYPE)
    {
      UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
      }
    }
    
    return CMD_TSKG_ERROR;
  }
  else
  {
    if((au8SendBuf[6] == 0) || (au8SendBuf[6] == 1))
    {
      if(au8SendBuf[6] != g_bDebug)
      {
        g_bDebug = au8SendBuf[6];
        g_u8NeedSaved |= SAVE_DEBUG_SW;  
      }
    
      u8CheckSum = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
      
       //���ͷ�����Ϣ
      if(u8Index == USB_TYPE)
      {
         UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
      }
      else
      {
        if(u8Index == DTU_TYPE)
        {
          UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
        }
      }
      
      return CMD_TSKG_OK;
    }
  }
return CMD_TSKG_ERROR;
}


/**********************************************************************************************
 �������ƣ� GetVersionProcess
 �������ܣ���ȡ�汾��Ϣ

 ������ u8Index   ��Ҫ������ͨ����USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 ���أ� У���״̬
**********************************************************************************************/
UINT8 GetVersionProcess(UINT8 u8Index, UINT8 *pu8RxBuffer)
{
  UINT8 i;
  UINT16 u16DataLen=0;
  UINT8 u8CheckSum=0;
  UINT8 au8SendBuf[20];

  if((u8Index != USB_TYPE) && (u8Index != DTU_TYPE))
  {
    return TYPE_ERROR;
  }
  
  //�����������ݰ���У��ͣ����ݳ���
  //���㷢��У���
  for(i=1; i<20; i++)
  {
    if(pu8RxBuffer[i] != '*')
    {
      u8CheckSum ^= pu8RxBuffer[i];
    }
    else
    {
      i++;
      break;
    }
  }

  u16DataLen = i+1;
    
  MEM_SET(au8SendBuf, 0, 20);
  MEM_CPY(au8SendBuf, pu8RxBuffer, u16DataLen + 1);
  
  //����BUf�����ʼ��
  au8SendBuf[1] = 'B';
  au8SendBuf[2] = 'B';
  au8SendBuf[3] = 'H';
  au8SendBuf[4] = 'M';
  
  //�ȶ�У����Ƿ���ȷ
  if(u8CheckSum != pu8RxBuffer[i])
  {
    //����״̬��ֵ
    au8SendBuf[5] = ',';
    au8SendBuf[6] = 0;
    au8SendBuf[7] = 13;
    au8SendBuf[8] = ',';
    au8SendBuf[9] = 0x00;
    au8SendBuf[10] = ',';
    au8SendBuf[11] = '*';
    
    //���㷢��У���
    au8SendBuf[12] = CalcCheckSum(au8SendBuf, 20);
    au8SendBuf[13] = 0x0D;
    
    if(u8Index == USB_TYPE)
    {
      UartSendBuffer(UART2, &au8SendBuf[0], 14);
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], 14);
      }
    }
    
    return CMD_BBHM_ERROR;
  }
  else
  {
     au8SendBuf[5] = ',';
     au8SendBuf[6] = 0;
     au8SendBuf[7] = 14;
     au8SendBuf[8] = ',';
     au8SendBuf[9] = VERSION_MAIN;
     au8SendBuf[10] = VERSION_SUB;
     au8SendBuf[11] = ',';
     au8SendBuf[12] = '*';
    
     au8SendBuf[13] = CalcCheckSum(au8SendBuf, 20);
     au8SendBuf[14] = 0x0d;
      
       //���ͷ�����Ϣ
     if(u8Index == USB_TYPE)
     {
        UartSendBuffer(UART2, &au8SendBuf[0], 15);
     }
     else
     {
       if(u8Index == DTU_TYPE)
       {
         UartSendBuffer(UART1, &au8SendBuf[0], 15);
       }
     }
      
     return CMD_BBHM_OK;
  }

}

/**********************************************************************************************
 �������ƣ� PowerDownProcess
 �������ܣ��ն˹ػ�ָ��

 ������ u8Index   ��Ҫ������ͨ����USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 ���أ� У���״̬
**********************************************************************************************/
UINT8 PowerDownProcess(UINT8 u8Index, UINT8 *pu8RxBuffer)
{
  UINT8 i;
  UINT16 u16DataLen=0;
  UINT8 u8CheckSum=0;
  UINT8 au8SendBuf[20];

  if((u8Index != USB_TYPE) && (u8Index != DTU_TYPE))
  {
    return TYPE_ERROR;
  }
  
  //�����������ݰ���У��ͣ����ݳ���
  //���㷢��У���
  for(i=1; i<20; i++)
  {
    if(pu8RxBuffer[i] != '*')
    {
      u8CheckSum ^= pu8RxBuffer[i];
    }
    else
    {
      i++;
      break;
    }
  }

  u16DataLen = i+1;
    
  MEM_SET(au8SendBuf, 0, 20);
  MEM_CPY(au8SendBuf, pu8RxBuffer, u16DataLen + 1);
  
  //����BUf�����ʼ��
  au8SendBuf[1] = 'Z';
  au8SendBuf[2] = 'D';
  au8SendBuf[3] = 'G';
  au8SendBuf[4] = 'J';
  
  //�ȶ�У����Ƿ���ȷ
  if(u8CheckSum != pu8RxBuffer[i])
  {
    //����״̬��ֵ
    au8SendBuf[5] = ',';
    au8SendBuf[6] = 0;
    au8SendBuf[7] = 13;
    au8SendBuf[8] = ',';
    au8SendBuf[9] = 0x00;
    au8SendBuf[10] = ',';
    au8SendBuf[11] = '*';
    
    //���㷢��У���
    au8SendBuf[12] = CalcCheckSum(au8SendBuf, 20);
    au8SendBuf[13] = 0x0D;
    
    if(u8Index == USB_TYPE)
    {
      UartSendBuffer(UART2, &au8SendBuf[0], 14);
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], 14);
      }
    }
    
    return CMD_ZDGJ_ERROR;
  }
  else
  {
     au8SendBuf[5] = ',';
     au8SendBuf[6] = 0;
     au8SendBuf[7] = 13;
     au8SendBuf[8] = ',';
     au8SendBuf[9] = 0x01;
     au8SendBuf[10] = ',';
     au8SendBuf[11] = '*';
    
     au8SendBuf[12] = CalcCheckSum(au8SendBuf, 20);
     au8SendBuf[13] = 0x0d;
      
       //���ͷ�����Ϣ
     if(u8Index == USB_TYPE)
     {
        UartSendBuffer(UART2, &au8SendBuf[0], 14);
     }
     else
     {
       if(u8Index == DTU_TYPE)
       {
         UartSendBuffer(UART1, &au8SendBuf[0], 14);
       }
     }
     
     //��ʾ�ػ�����
    DisplayOffPage();
    //�ر�DTU��Դ
    DtuPowerOff();
    GpsPowerOff();
    //�ر�OLED��Դ
    OledPowerDown();
    //�ر�ϵͳ��Դ
    SystemPowerOff();
    OpenWatchDog(); // �򿪿��Ź�
    while(1);
      
//     return CMD_ZDGJ_OK;
  }

}

/**********************************************************************************************
 �������ƣ� ReadDtuConfigProcess
 �������ܣ���ȡDTU�ı�ʾ��

 ������ u8Index   ��Ҫ������ͨ����USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 ���أ� У���״̬
**********************************************************************************************/
UINT8 ReadDtuConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer)
{
   UINT8 i;
  UINT16 u16DataLen=0;
  UINT8 u8CheckSum=0;
  UINT8 au8SendBuf[24];

  if((u8Index != USB_TYPE) && (u8Index != DTU_TYPE))
  {
    return TYPE_ERROR;
  }
  
  //�����������ݰ���У��ͣ����ݳ���
  //���㷢��У���
  for(i=1; i<20; i++)
  {
    if(pu8RxBuffer[i] != '*')
    {
      u8CheckSum ^= pu8RxBuffer[i];
    }
    else
    {
      i++;
      break;
    }
  }

  u16DataLen = i+1;
    
  MEM_SET(au8SendBuf, 0, 20);
  MEM_CPY(au8SendBuf, pu8RxBuffer, u16DataLen + 1);
  
  //����BUf�����ʼ��
  au8SendBuf[1] = 'Y';
  au8SendBuf[2] = 'J';
  au8SendBuf[3] = 'X';
  au8SendBuf[4] = 'X';
  
  //�ȶ�У����Ƿ���ȷ
  if(u8CheckSum != pu8RxBuffer[i])
  {
    au8SendBuf[5] = ',';
    au8SendBuf[6] = 0;
    au8SendBuf[7] = 13;
    //����״̬��ֵ
    au8SendBuf[8] = ',';
    au8SendBuf[9] = 0x00;
    au8SendBuf[10] = ',';
    au8SendBuf[11] = '*';
    
    //���㷢��У���
    au8SendBuf[12] = CalcCheckSum(au8SendBuf, 20);
    au8SendBuf[13] = 0x0D;
    
    if(u8Index == USB_TYPE)
    {
      UartSendBuffer(UART2, &au8SendBuf[0], 14);
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], 14);
      }
    }
    
    return CMD_DTHM_ERROR;
  }
  else
  {
     au8SendBuf[5] = ',';
     au8SendBuf[6] = 0;
     au8SendBuf[7] = 23;
     au8SendBuf[8] = ',';
     MEM_CPY(&au8SendBuf[9], &sMainServer.au8BckId[0], 11);
     au8SendBuf[20] = ',';
     au8SendBuf[21] = '*';
    
     au8SendBuf[22] = CalcCheckSum(au8SendBuf, 20);
     au8SendBuf[23] = 0x0d;
      
       //���ͷ�����Ϣ
     if(u8Index == USB_TYPE)
     {
        UartSendBuffer(UART2, &au8SendBuf[0], 24);
     }
     else
     {
       if(u8Index == DTU_TYPE)
       {
         UartSendBuffer(UART1, &au8SendBuf[0], 24);
       }
     }
     

     return CMD_DTHM_OK;
  }

}

/**********************************************************************************************
 �������ƣ�ConfigPackType
 �������ܣ������������

 ������ ��
 ���أ� ��������
**********************************************************************************************/
SINT8 ConfigPackType(const UINT8 *pu8Buf, UINT8 u8Size)
{
  if(u8Size < 5)
    return PZNON;
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[0], 5))
    return XXPZ;    
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[1], 5))
    return DMPZ;
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[2], 5))
    return S1PZ;
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[3], 5))
    return S2PZ;
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[4], 5))
    return S3PZ;
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[5], 5))
    return S4PZ;
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[6], 5))
    return ZDQD;
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[7], 5))
    return ZDTZ;
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[8], 5))
    return ZDGJ;
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[9], 5))
    return TSKG;
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[10], 5))
    return BBHM;
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[11], 5))
    return YJCX;

  return PZNON;
}
/* ====================================================================================================== */

/***************************************************************************
//�������ƣ�UsbCmdInit
//����������USBת�������������ʼ��
//
//��������
//���أ���
***************************************************************************/
void UsbCmdInit(void)
{
  sUartCmdCb[USB_TYPE].eAssembleState = E_UART_CMD_RECEIVING_HEADER;
  sUartCmdCb[USB_TYPE].pu8AssembleBuf = (UINT8 *)au8UsbBodyBuf;
  sUartCmdCb[USB_TYPE].u8BufDataLength = 0;

  //����0���ճ�ʱ��ʱ����
  asTimer[TIMER_UART2].u16LifeTime = TIMER_TICK / 2;
  asTimer[TIMER_UART2].u16LifeOldTime = TIMER_TICK / 2;
  asTimer[TIMER_UART2].bEnable = FALSE;
}

/***********************************************************************************************
  �������ƣ�UartCmdAssemble(UINT8 u8InData)
  �������ܣ��ѽ��յ�������ϳ���Ч�����ݰ�

  ���룺u8InData
  �������
***********************************************************************************************/
void Uart2CmdAssemble(UINT8 u8InData)
{
  switch(sUartCmdCb[USB_TYPE].eAssembleState)
  {
    case E_UART_CMD_RECEIVING_HEADER:
    {
      if(u8InData == '$')
      {
        sUartCmdCb[USB_TYPE].pu8AssembleBuf[0] = u8InData;
        sUartCmdCb[USB_TYPE].u8BufDataLength = 1;
	      sUartCmdCb[USB_TYPE].eAssembleState = E_UART_CMD_RECEIVING_BODY;	
      }
      break;
     }	
		
    case E_UART_CMD_RECEIVING_BODY:
    {
      sUartCmdCb[USB_TYPE].pu8AssembleBuf[sUartCmdCb[USB_TYPE].u8BufDataLength++] = u8InData;
      if((u8InData == '\r') && (sUartCmdCb[USB_TYPE].pu8AssembleBuf[sUartCmdCb[USB_TYPE].u8BufDataLength-3] == '*'))
      {
        sUartCmdCb[USB_TYPE].eAssembleState = E_UART_CMD_RECEIVING_VALID;
      }
      else
      {
        if(sUartCmdCb[USB_TYPE].u8BufDataLength == MAX_BODY_SIZE)	
        {
          sUartCmdCb[USB_TYPE].eAssembleState = E_UART_CMD_RECEIVING_HEADER;
          sUartCmdCb[USB_TYPE].u8BufDataLength = 0;
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
      sUartCmdCb[USB_TYPE].eAssembleState = E_UART_CMD_RECEIVING_HEADER;
      sUartCmdCb[USB_TYPE].u8BufDataLength = 0;
    }
  }	
}


/***************************************************************************
//�������ƣ�UsbCmdRun
//����������USBת���������������
//
//��������
//���أ���
***************************************************************************/
void UsbCmdRun(void)
{
  UINT8 u8Cmd;
  UINT8 u8CheckSumState=CMD_NONE;

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
      
      switch(u8Cmd)
      {
        case XXPZ:
        {
          u8CheckSumState = ConfigInfoProcess(USB_TYPE, au8UsbBodyBuf);   //��ȡ������Ϣ
          break;
        }
        
        case DMPZ:
        {
          u8CheckSumState = MainServerConfigProcess(USB_TYPE, au8UsbBodyBuf);   //��ȡ������Ϣ
          break;
        }
        
        case S1PZ:
        {
          u8CheckSumState = Sub1ConfigProcess(USB_TYPE, au8UsbBodyBuf);   //��ȡ������Ϣ
          break;
        }
        
        case S2PZ:
        {
          u8CheckSumState = Sub2ConfigProcess(USB_TYPE, au8UsbBodyBuf);   //��ȡ������Ϣ
          break;
        }
        
        case S3PZ:
        {
          u8CheckSumState = Sub3ConfigProcess(USB_TYPE, au8UsbBodyBuf);   //��ȡ������Ϣ
          break;
        }
        
        case S4PZ:
        {
          u8CheckSumState = Sub4ConfigProcess(USB_TYPE, au8UsbBodyBuf);   //��ȡ������Ϣ
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
          u8CheckSumState = PowerDownProcess(USB_TYPE, au8UsbBodyBuf);          
          break;
        }
        
        case TSKG:
        {
          u8CheckSumState = DebugProcess(USB_TYPE, au8UsbBodyBuf);
          break;
        }
        
        case BBHM:
        {
          u8CheckSumState = GetVersionProcess(USB_TYPE, au8UsbBodyBuf);
          break;
        }
        
        case YJCX:
        {
          u8CheckSumState = ReadDtuConfigProcess(USB_TYPE, au8UsbBodyBuf);
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
          UartSendString(UART2, "��Ϣ�������óɹ���\r\n");
          break;
        }
        
        case CMD_XXPZ_ERROR:
        {
          UartSendString(UART2, "��Ϣ��������ʧ�ܣ�\r\n");
          break;
        }
        
        case CMD_DMPZ_OK:
        {
          UartSendString(UART2, "���������������óɹ���\r\n");
          break;
        }
        
        case CMD_DMPZ_ERROR:
        {
          UartSendString(UART2, "����������������ʧ�ܣ�\r\n");
          break;
        }
        
        case CMD_S1PZ_OK:
        {
          UartSendString(UART2, "������1�������óɹ���\r\n");
          break;
        }
        
        case CMD_S1PZ_ERROR:
        {
          UartSendString(UART2, "������1��������ʧ�ܣ�\r\n");
          break;
        }
        
        case CMD_S2PZ_OK:
        {
          UartSendString(UART2, "������2�������óɹ���\r\n");
          break;
        }
        
        case CMD_S2PZ_ERROR:
        {
          UartSendString(UART2, "������2��������ʧ�ܣ�\r\n");
          break;
        }
        
        case CMD_S3PZ_OK:
        {
          UartSendString(UART2, "������3�������óɹ���\r\n");
          break;
        }
        
        case CMD_S3PZ_ERROR:
        {
          UartSendString(UART2, "������3��������ʧ�ܣ�\r\n");
          break;
        }
        
        case CMD_S4PZ_OK:
        {
          UartSendString(UART2, "������4�������óɹ���\r\n");
          break;
        }
        
        case CMD_S4PZ_ERROR:
        {
          UartSendString(UART2, "������4��������ʧ�ܣ�\r\n");
          break;
        }
        
        case CMD_TSKG_OK:
        {
          UartSendString(UART2, "���Կ������óɹ���\r\n");
          break;
        }
        
        case CMD_TSKG_ERROR:
        {
          UartSendString(UART2, "���Կ�������ʧ�ܣ�\r\n");
          break;
        }
        
        case CMD_BBHM_OK:
        {
          UartSendString(UART2, "�̼��汾��ȡ�ɹ���\r\n");
          break;
        }
        
        case CMD_BBHM_ERROR:
        {
          UartSendString(UART2, "�̼��汾��ȡʧ�ܣ�\r\n");
          break;
        }
        
        case CMD_DTHM_OK:
        {
          UartSendString(UART2, "DTU��ʶ��ȡ�ɹ���\r\n");
          break;
        }
        
        case CMD_DTHM_ERROR:
        {
          UartSendString(UART2, "DTU��ʶ��ȡʧ�ܣ�\r\n");
          break;
        }
        
        default: break;
      }
    }
        
      //���������ָ����buf
      MEM_SET(&au8UsbBodyBuf[0], 0, sizeof(au8UsbBodyBuf));
      sUartCmdCb[USB_TYPE].eAssembleState = E_UART_CMD_RECEIVING_HEADER;
      sUartCmdCb[USB_TYPE].u8BufDataLength = 0;
    }
    else
    {
      //�涨ʱ���������ݣ�ָ�λ��������ȡ��Ч����
      if(asTimer[TIMER_UART2].bEnable == TRUE)                
      {
        if(asTimer[TIMER_UART2].u16LifeTime == 0)
        {
          sUartCmdCb[USB_TYPE].eAssembleState = E_UART_CMD_RECEIVING_HEADER;
          sUartCmdCb[USB_TYPE].u8BufDataLength = 0;

          asTimer[TIMER_UART2].bEnable = FALSE;
        }	
       }
       else
       {
          if(sUartCmdCb[USB_TYPE].u8BufDataLength != 0)	
          {
            asTimer[TIMER_UART2].u16LifeTime = asTimer[TIMER_UART2].u16LifeOldTime;
            asTimer[TIMER_UART2].bEnable = TRUE;
          }
        }	
     }
}



/***************************************************************************
//�������ƣ�UartCmdInit
//�������������ڽ����������
//
//��������
//���أ���
***************************************************************************/
void UartCmdInit(void)
{
  /* gps���������ʼ�� */
  GpsCmdInit();
  
  /* DTU���������ʼ�� */
  DtuCmdInit();
  
  /* USBת�������������ʼ�� */
  UsbCmdInit();
  
  /* ����ģ�����������ʼ�� */
//  WirelessCmdInit();
}

/***************************************************************************
//�������ƣ�UartCmdRun
//�������������ڽ����������
//
//��������
//���أ���
***************************************************************************/
void UartCmdRun(void)
{
  /* gps������� */
  GpsCmdRun();
  
  /* DTU������� */
  DtuCmdRun();
  
  /* usbת����������� */
  UsbCmdRun();
  
  /* ���ߴ���������� */
//  WirelessCmdRun();       //��ʱû��ʹ�ã���������
  
  /* DTU��Ϣ���� */
  DtuInfoSend();          //���ɼ������������ݰ�ASCII����ʽ���͸�����
}