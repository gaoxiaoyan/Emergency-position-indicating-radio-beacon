/******************************************************************************
*											
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved										
*
* �� �� ���� GpsParse.C
* ��    ����GPS���ݽ���

* �������ڣ� 2014��9��5��11:31:59
* ��    �ߣ� Bob
* ��ǰ�汾�� V1.00

------------------------------------------------------
*******************************************************************************/

#define   GPS_GLOBALS
#include "GpsParse.h"
#include "string.h"
#include "ctype.h"
#include "global.h"
#include "Timer.h"
#include "Uart2.h"
#include "Uart3.h"
#include "SystemCtl.h"
#include "OS_MEM.h"
#include "SoftTimer.h"
#include "Rtc.h"
#include "Communication.h"

#define GPS_BUF_SIZE    BUF_LARGE_SIZE



//GPS ������Ϣ�ṹ����
typedef struct
{
  //��λ��Ч��־
  BOOL bActive;

  // UTCʱ��
//  UINT8 UTC_Time[6];
//  UINT8 UTC_Date[6];

  // γ��
  UINT8 GPS_Y[10];
  //γ�Ȱ���
  UINT8 NorthSouth;
  
  // ����
  UINT8 GPS_X[11];
  // ���Ȱ���
  UINT8 EastWest;

  //�������ʣ�000.0 ~999.9��   ******************************************/
//  UINT8 Speed[8];

  //���溽��000.0~359.9�� ���汱����Ϊ��׼��
//  UINT8 Course[8];

  //ʹ����������
  UINT8 UseEphSum;

}_tsGpsInfo;


#define GPS_MAX_SIZE      8
//�ⲿ�ṹ��������Ҫ�������ջ�����������
GPS_EXT _tsGpsInfo sGpsInfoBuf[GPS_MAX_SIZE];


//���ݰ�����Э��
typedef enum
{
  //�ȴ����ݵ���ʼ��־
  E_UART_CMD_RECEIVING_HEADER,
  //��������
  E_UART_CMD_RECEIVING_BODY,
  //������Ч
  E_UART_CMD_RECEIVING_VALID
		
}teUartCmdAssembleState;

/* ��������ṹ���� */
typedef struct
{
  UINT8 *pu8AssembleBuf;  //���������ָ��
  
  UINT8 *pu8CmdIndex;     //��������ָ��

//  BOOL bGpsParse;         //���ݽ�����־��TRUE--���ڽ�����FALSE -- ������� 
		
  UINT16 u16BufDataLen ;  //��������ݳ���
	
  teUartCmdAssembleState eAssembleState;  //���״̬
  
  SINT8 sGpsDog;
	
}_tsUartCmdCb;

_tsUartCmdCb sGpsCmdCb;

//GPS��������
const UINT8 *pGpsCmd[] = {"$GNGGA", "$GNGLL","$GNRMC", "$GNVTG"};
//GPS ��������
enum eNmeaType
{
  GNNON = 0x00,
  GNGGA = 0x01,
  GNGLL = 0x02,
  GNRMC = 0x04,
  GNVTG = 0x08
};

#define GPS_RX_INVALID      0
#define GPS_RX_VALID        1
#define GPS_RX_COMPLETE     2

//gps module gngga cmd index
#define GPS_UTC_TIME        1
#define GPS_LOC_Y           2
#define GPS_N_S             3
#define GPS_LOC_X           4
#define GPS_E_W             5
#define GPS_VALID           6
#define GPS_STAR            7
#define GPS_CHECK           13

//һ�����·��б�
const UINT8 au8Leap[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const UINT8 au8Year[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#define RtcHex2Bcd(u8Hex) ((((u8Hex % 100) / 10) << 4) + ((u8Hex % 100) % 10))
#define Bcd2Hex(u8Bcd) ((((u8Bcd & 0xf0) >> 4) % 10) * 10 + ((u8Bcd & 0x0f) % 10))

UINT8 u8Num=0;
BOOL bRtcFlag=FALSE;


/**********************************************************************************************
 �������ƣ�NmeaPackType
 �������ܣ�GPS �������

 ������ ��
 ���أ� 0183Э����������
**********************************************************************************************/
SINT8 NmeaPackType(const UINT8 *pu8Buf, UINT8 u8Size)
{
  if(u8Size < 6)
    return GNNON;
  else if(0 == memcmp(pu8Buf, pGpsCmd[0], 6))
    return GNGGA;
  else if(0 == memcmp(pu8Buf, pGpsCmd[1], 6))
    return GNGLL;
  else if(0 == memcmp(pu8Buf, pGpsCmd[2], 6))
    return GNRMC;

  return GNNON;
}

/**********************************************************************************************
 �������ƣ�CreatCmdIndex
 �������ܣ�������������

 ������ ��
 ���أ� ��
**********************************************************************************************/
void CreatCmdIndex(void)
{
  UINT8 i,j=0;
  
  memset(sGpsCmdCb.pu8CmdIndex, 0, BUF_SMALL_SIZE);
  
  //��ȡ��ָ���������
  for(i=0; i<sGpsCmdCb.u16BufDataLen; i++)
  {
     if(sGpsCmdCb.pu8AssembleBuf[i] == ',')
     {
       sGpsCmdCb.pu8CmdIndex[j++] = i;
     }
   }
}

/***********************************************************************************
  �������ƣ�RealCmdProcess
  �������ܣ������������ݵ�N��������ƫ��

  ���룺��Ҫ��ȡ�Ĳ���λ�ñ���
  ��������ص�N��","֮�����Ϣ����Ҫ*buffer��Ч������������ſ���ִ��
***********************************************************************************/
UINT8* RealCmdProcess(UINT8 num )
{
  if ( num < 1 )
    return  &sGpsCmdCb.pu8AssembleBuf[0];
  return  &sGpsCmdCb.pu8AssembleBuf[ sGpsCmdCb.pu8CmdIndex[num - 1] + 1];
}


/**********************************************************************************************
 �������ƣ�GpsInfoProcess
 �������ܣ�����GPS��Ч����

 ������ GPS�������
 ���أ� ��
**********************************************************************************************/
UINT8 GpsInfoProcess(UINT8 u8Cmd)
{
  UINT8 * str;
  
  //GPGGA ָ�����
  if(u8Cmd == GNGGA )
  {
    if( *RealCmdProcess(GPS_VALID) != '0')  //��6������Ϊ��λ��Чλ
    {
      sGpsInfoBuf[u8Num].bActive = GPS_RX_VALID;
    }
    else
    {
      if(g_bDebug & GPS_DEBUG)
      {
        Uart3SendString("#Gps Receiving Invalid Data!\r\n");
      }
      
      return GPS_RX_INVALID;
    }
    
    //��λ��Чʱ��ȡ��γ����Ϣ
    if(sGpsInfoBuf[u8Num].bActive == GPS_RX_VALID)
    {
 //     str = RealCmdProcess(GPS_UTC_TIME);                             //��1������Ϊʱ��
 //     memcpy(pGpsInfoBuf[u8Num]->GPS_Y, str, 10);

      str = RealCmdProcess(GPS_LOC_Y);                                  //��2������Ϊά��
      memcpy(sGpsInfoBuf[u8Num].GPS_Y, str, 10);
//      sGpsInfoBuf[u8Num].GPS_Y[10] = 0;

      sGpsInfoBuf[u8Num].NorthSouth = *RealCmdProcess(GPS_N_S);         //��3������Ϊ�ϱ�����

      str = RealCmdProcess(GPS_LOC_X);                                  //��4������Ϊ����
      memcpy(sGpsInfoBuf[u8Num].GPS_X, str, 11);
 //     sGpsInfoBuf[u8Num].GPS_X[11] = 0;

      sGpsInfoBuf[u8Num].EastWest = *RealCmdProcess(GPS_E_W);           //��5������Ϊ��������

      sGpsInfoBuf[u8Num].UseEphSum = (UINT8)(atof(RealCmdProcess(GPS_STAR)));    //GPGGA��7������Ϊ���Ǹ���
                
      if(g_bDebug & GPS_DEBUG)
      {
        Uart3SendString("#========================================================================#\r\n");
        Uart3SendString("#The Current Position is: γ�� <");
        Uart3SendBuffer(sGpsInfoBuf[u8Num].GPS_Y, 11);
        Uart3SendString(" >...���� < ");
        Uart3SendBuffer(sGpsInfoBuf[u8Num].GPS_X, 12);
        Uart3SendString(" >....#\r\n");
        Uart3SendString("#========================================================================#\r\n");
      }
    
      u8Num++;
      if(u8Num == GPS_MAX_SIZE)
      {
        u8Num = 0;
        if(g_bDebug & GPS_DEBUG)
        {
          Uart3SendString("#Gps Receiving Data Complete!\r\n");
        }
        
        return GPS_RX_COMPLETE;
      }
    }
    
    return GPS_RX_VALID;
  }
  
  return GPS_RX_VALID;
}

/**********************************************************************************************
 �������ƣ�StringToUint32
 �������ܣ�

 ������ ��
 ���أ� UINT32
**********************************************************************************************/
UINT32 StringToUint32(UINT8 *ptr)
{
  UINT32 u32Temp=0;
  
  u32Temp += (UINT32)(ptr[0] - '0') * 10000;
  u32Temp += (UINT32)(ptr[1] - '0') * 1000;
  u32Temp += (UINT32)(ptr[2] - '0') * 100;
  u32Temp += (UINT32)(ptr[3] - '0') * 10;
  u32Temp += (UINT32)(ptr[4] - '0');
  
  return u32Temp;
}

/**********************************************************************************************
 �������ƣ�Uint32ToString
 �������ܣ�

 ������ UINT32
 ���أ� *ptr
**********************************************************************************************/
void Uint32ToString(UINT8 *ptr, UINT32 u32Temp)
{
  ptr[0] = u32Temp / 10000 + '0';
  ptr[1] = u32Temp % 10000 / 1000 + '0';
  ptr[2] = u32Temp % 1000 /100 + '0';
  ptr[3] = u32Temp %100 /10 + '0';
  ptr[4] = u32Temp % 10 + '0';
}

/**********************************************************************************************
 �������ƣ�CalibratLocation
 �������ܣ�У׼GPS���ݣ�ȡƽ��ֵ��Ϊ���Ķ�λ��γ��

 ������ ��
 ���أ� ��
**********************************************************************************************/
void CalibratLocation(void)
{
  UINT8 i=0;
  UINT32 u32X=0;
  UINT32 u32Y=0;
  UINT8 au8Buf[5];
  
  for(i=0; i<GPS_MAX_SIZE; i++)
  {
    u32Y += StringToUint32(&sGpsInfoBuf[i].GPS_Y[5]);
  }
  u32Y = u32Y / GPS_MAX_SIZE;
  Uint32ToString(au8Buf, u32Y);
  memcpy(&sGpsInfoBuf[GPS_MAX_SIZE-1].GPS_Y[5], au8Buf, 5);
  
  for(i=0; i<GPS_MAX_SIZE; i++)
  {
    u32X += StringToUint32(&sGpsInfoBuf[i].GPS_X[6]);
  }
  u32X = u32X / GPS_MAX_SIZE;
  Uint32ToString(au8Buf, u32X);
  memcpy(&sGpsInfoBuf[GPS_MAX_SIZE-1].GPS_X[6], au8Buf, 5);
  
  sSendInfo.pGPS_Y = sGpsInfoBuf[GPS_MAX_SIZE-1].GPS_Y;
  sSendInfo.pGPS_X = sGpsInfoBuf[GPS_MAX_SIZE-1].GPS_X;
}


/**********************************************************************************************
 �������ƣ�CalibratRtc
 �������ܣ�У׼RTC

 ������ u8Cmd
 ���أ� ��
**********************************************************************************************/
BOOL CalibratRtc(UINT8 u8Cmd)
{
  UINT8 *ptr;
  UINT8 au8Time[6];
  UINT8 au8Date[6];
  _tsTimeCb sTime;
  UINT16 Year;
  UINT8 Temp;
  
  //GNRMC ָ�����
  if(u8Cmd == GNRMC )
  {
    if( *RealCmdProcess(2) == 'A')  //��2������Ϊ������Чλ
    {
      ptr = RealCmdProcess(1);           //utcʱ��
      memcpy(au8Time, ptr, 6);
      
      ptr = RealCmdProcess(9);           //utc����
      memcpy(au8Date, ptr, 6);
      
      //string to time
      sTime.Hour   = (au8Time[0] - 0x30)*10 + (au8Time[1] - 0x30) + 8;
      sTime.Minute = (au8Time[2] - 0x30)*10 + (au8Time[3] - 0x30);
      sTime.Second = (au8Time[4] - 0x30)*10 + (au8Time[5] - 0x30);

      sTime.Date  = (au8Date[0] - 0x30)*10 + (au8Date[1] - 0x30);
      sTime.Month = (au8Date[2] - 0x30)*10 + (au8Date[3] - 0x30);
      sTime.Year  = (au8Date[4] - 0x30)*10 + (au8Date[5] - 0x30);
  
      //UTCʱ��=>����ʱ��ת��
      if(sTime.Hour > 23)
      {
        sTime.Hour -= 24;
        sTime.Date += 1;

        Year = sTime.Year + 2000;
        if(((Year%4 == 0) && (Year%100!=0)) || (Year%400 == 0))
        {
          Temp = au8Leap[sTime.Month];
        }
        else
        {
          Temp = au8Year[sTime.Month];
        }

        if(Temp < sTime.Date)
        {
          sTime.Date = 1;
          sTime.Month += 1;
          if(sTime.Month >12 )
          {
            sTime.Month = 1;
            sTime.Year += 1;
          }
        }
      }
      
      //set time to rtc
      SetTimeToRtc(&sTime);
      
      return TRUE;
    }
    
    return FALSE;
  }
  
  return FALSE;
}

/***********************************************************************************************
  �������ƣ�GpsCmdAssemble(UINT8 u8InData)
  �������ܣ��ѽ��յ�������ϳ���Ч�����ݰ�

  ���룺u8InData
  �������
***********************************************************************************************/
void GpsCmdAssemble(UINT8 u8InData)
{
  switch(sGpsCmdCb.eAssembleState)
  {
    case E_UART_CMD_RECEIVING_HEADER:
    {
      if(u8InData == '$')
      {
        sGpsCmdCb.pu8AssembleBuf[0] = u8InData;
        sGpsCmdCb.u16BufDataLen = 1;
	      sGpsCmdCb.eAssembleState = E_UART_CMD_RECEIVING_BODY;	
      }
      break;
     }	
		
    case E_UART_CMD_RECEIVING_BODY:
    {
      sGpsCmdCb.pu8AssembleBuf[sGpsCmdCb.u16BufDataLen++] = u8InData;
      if(u8InData == '\r')
      {
        sGpsCmdCb.eAssembleState = E_UART_CMD_RECEIVING_VALID;
      }
      else
      {
        if(sGpsCmdCb.u16BufDataLen == GPS_BUF_SIZE)	
        {
          sGpsCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
          sGpsCmdCb.u16BufDataLen = 0;
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
      sGpsCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
      sGpsCmdCb.u16BufDataLen = 0;
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
  //  sGpsCmdCb.bGpsParse = FALSE;
  sGpsCmdCb.u16BufDataLen = 0;
  sGpsCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
  sGpsCmdCb.pu8AssembleBuf = OSMemGet(pSmallMem, &err);
  sGpsCmdCb.pu8CmdIndex = OSMemGet(pSmallMem, &err);
  //Get One soft timer
  sGpsCmdCb.sGpsDog = SwdGet();   
  SwdSetLifeTime(sGpsCmdCb.sGpsDog, 200);
}

/**********************************************************************************************
 �������ƣ�GpsCmdRun
 �������ܣ�GPS ��������������ɼ�8����Ч���ݣ��ҳ����������õ�һ������

 ������ ��
 ���أ� ��
**********************************************************************************************/
void GpsCmdRun(void)
{
  UINT8 u8Cmd=GNNON;
  UINT8 GpsStatus=GPS_RX_INVALID;
  
 //��ȡ��Ч���ݰ�
  while((sGpsCmdCb.eAssembleState != E_UART_CMD_RECEIVING_VALID) && (Uart2GetRxDataLength() > 0))	
  {
    GpsCmdAssemble(Uart2GetByte());

    if(SwdEnable(sGpsCmdCb.sGpsDog) == TRUE)
    {
      SwdDisable(sGpsCmdCb.sGpsDog);
    }
  }
    
  if(sGpsCmdCb.eAssembleState == E_UART_CMD_RECEIVING_VALID)
  {
    u8Cmd = NmeaPackType(sGpsCmdCb.pu8AssembleBuf, sGpsCmdCb.u16BufDataLen);
    if(u8Cmd != GNNON)
    {
      if(g_bDebug & GPS_DEBUG)    //��ӡ������Ϣ
      {
        Uart3SendBuffer(sGpsCmdCb.pu8AssembleBuf, sGpsCmdCb.u16BufDataLen);
      }
      //����������
      CreatCmdIndex();
      //У׼RTC
      if(bRtcFlag == FALSE)
      {
        bRtcFlag = CalibratRtc(u8Cmd);
      }
      //Gps���ݴ���
      GpsStatus = GpsInfoProcess(u8Cmd);
      if(GpsStatus == GPS_RX_COMPLETE)
      {
        //��ȡ������Ч��γ��
        CalibratLocation();
        bRtcFlag = FALSE;
        
        g_bGpsCompleted = TRUE;
       
        //�رմ���2����ս��ջ�����
        Uart2DisableInterrupt();
        Uart2EmptyRxBuf();
      }
    }
    
    memset(sGpsCmdCb.pu8AssembleBuf, 0, BUF_SMALL_SIZE);
    sGpsCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
    sGpsCmdCb.u16BufDataLen = 0;
  }
  else
  {
    if(SwdEnable(sGpsCmdCb.sGpsDog) == TRUE)
    {
      if(SwdGetLifeTime(sGpsCmdCb.sGpsDog) == 0)
      {
        sGpsCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
        sGpsCmdCb.u16BufDataLen = 0;
        
        SwdReset(sGpsCmdCb.sGpsDog);
        SwdDisable(sGpsCmdCb.sGpsDog);
      }
    }
    else
    {
      if(sGpsCmdCb.u16BufDataLen != 0)
      {
        SwdReset(sGpsCmdCb.sGpsDog);
        SwdEnable(sGpsCmdCb.sGpsDog);
      }
    }
  }
}