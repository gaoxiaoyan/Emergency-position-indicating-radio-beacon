/******************************************************************************
*
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved
*
* �� �� ���� Rtc.c
* ��       ����

* �������ڣ� 2014��11��12��16:15:25
* ��    �ߣ� Bob
* ��ǰ�汾�� V1.00

------------------------------------------------------
*******************************************************************************/
#define  RTC_GLOBAL
#include "msp430x54x.h"
#include "global.h"
#include "DataType.h"
#include "Rtc.h"
#include "I2c_Driver.h"
#include "string.h"
#include "SoftTimer.h"

#define RtcBcd2Hex(u8Bcd) ((((u8Bcd & 0xf0) >> 4) % 10) * 10 + ((u8Bcd & 0x0f) % 10))
#define RtcHex2Bcd(u8Hex) ((((u8Hex % 100) / 10) << 4) + ((u8Hex % 100) % 10))

void SetTimeToRtc(_tsTimeCb * psTime);

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

//�ֽڸߵ�λ�任
UINT8 ByteExchange(UINT8 u8Data)
{
  UINT8 u8Temp;

  u8Temp = u8Data;
  u8Temp = ((u8Temp & 0x55) << 1) | ((u8Temp & 0xaa) >> 1);
  u8Temp = ((u8Temp & 0x33) << 2) | ((u8Temp & 0xcc) >> 2);
  u8Temp = ((u8Temp & 0x0f) << 4) | ((u8Temp & 0xf0) >> 4);

  return (u8Temp);
}

/*************************************************************************************************************
 * �������ƣ� RtcRest
 * ���������� ��λRTCоƬ
 * ��  �룺
 * ��  ����
 ************************************************************************************************************/
void RtcRest(void)
{
  UINT8 u8Cmd=0;

  u8Cmd |= STAT1_RESET;
  RtcWrite(CMD_STAT_1, &u8Cmd, 1);
  Delay(10);
}

#if 1
/*************************************************************************************************************
 * �������ƣ� RtcConverHour
 * ���������� 12/24��������
 * ��  �룺 bConver 1-- 24���ƣ� 0 -- 12����
 * ��  ����
 ************************************************************************************************************/
void RtcConverHour(BOOL bConver)
{
  UINT8 u8Cmd=0;

  if(bConver == TRUE)
    u8Cmd |= STAT1_CONVER;
  else
    u8Cmd &= ~STAT1_CONVER;

  RtcWrite(CMD_STAT_1, &u8Cmd, 1);
  Delay(10);
  //����һ�ζ�����������������Ϊ24��ʾ
  RtcRead(CMD_STAT_1, &u8Cmd, 1);
  Delay(10);
}
#endif

/*************************************************************************************************************
 * �������ƣ� SetRtcInt1Out
 * ���������� �����ж�1Ƶ�����, ���жϣ����ش���
 * ��  �룺
 * ��  ����
 ************************************************************************************************************/
void SetRtcInt1Out(void)
{
  UINT8 u8Cmd=0;

  RtcWrite(CMD_STAT_2, &u8Cmd, 1);
  Delay(5);

  u8Cmd |= STAT2_INT1ME;

  RtcWrite(CMD_STAT_2, &u8Cmd, 1);
  Delay(5);

/*  u8Cmd |= BIT7;
  RtcWrite(CMD_ALARM_1, &u8Cmd, 1);
  Delay(10);

  u8Cmd[0] = 0;
  RtcWrite(CMD_STAT_2, &u8Cmd[0], 1);
  Delay(10);

  u8Cmd[0] = STAT2_INT1AE;
  RtcWrite(CMD_STAT_2, &u8Cmd[0], 1);
  Delay(10);

  u8Cmd[0]  = 0;
  u8Cmd[1] = ByteExchange(0x15) | A1HE | BIT1;
  u8Cmd[2] = ByteExchange(0x12) | A1ME;
  RtcWrite(CMD_ALARM_1, &u8Cmd[0], 3);
  Delay(10);  */
}

/*************************************************************************************************************
 * �������ƣ� RtcInit
 * ���������� RTC��ʼ��
 * ��  �룺 psTime -- ��ǰʱ��ṹ
 * ��  ����
 ************************************************************************************************************/
void RtcInit(void)
{
  //RTC�жϣ��½��ش����ж�
  RtcIntIesEdge();
  RtcIntDir &= ~RtcInt;

//  RtcConverHour(1);

  SetRtcInt1Out();

  GetTimeFromRtc(&sCurTime);
}

/*************************************************************************************************************
 * �������ƣ� CaculateWeek
 * ���������� ���������ռ������ڣ�ʹ��̩���㷨
 * ��  �룺
 * ��  ����
 ************************************************************************************************************/
UINT8 CaculateWeek(UINT8 year, UINT8 month, UINT8 date)
{
  UINT8 u8Temp;

  if(month==1)
  {
    year-=1;
    month=13;
  }
  else
  {
    if(month==2)
    {
      year-=1;
      month=14;
    }
  }

  u8Temp = year+year/4+13*((month+1)/5)+date-36;
  u8Temp %= 7;

  return(u8Temp);
}

/*************************************************************************************************************
 * �������ƣ� GetTimeFromRtc
 * ���������� ��RTC�ж�����ǰ�����ں�ʱ��
 * ��  �룺
 * ��  ���� psTime -- ��ǰʱ��ṹ
 ************************************************************************************************************/
void GetTimeFromRtc(_tsTimeCb * psTime)
{
  UINT8 au8Temp[8];

  //��RTC���ԭʼ����
  RtcRead(CMD_YEAR, &au8Temp[0], 7);

  psTime->Year  = RtcBcd2Hex( ByteExchange(au8Temp[0]));
  psTime->Month = RtcBcd2Hex( ByteExchange(au8Temp[1]) & 0x1f);
  psTime->Date  = RtcBcd2Hex( ByteExchange(au8Temp[2]) & 0x3f);

  psTime->Week  = CaculateWeek(psTime->Year, psTime->Month, psTime->Date);

  psTime->Hour   = RtcBcd2Hex( ByteExchange(au8Temp[4]) & 0x3f);
//  RtcIntIeDisable();
  psTime->Minute = RtcBcd2Hex( ByteExchange(au8Temp[5]) & 0x7f);
//  RtcIntIeEnable();
  psTime->Second = RtcBcd2Hex( ByteExchange(au8Temp[6]) & 0x7f);
}

/*************************************************************************************************************
 * �������ƣ� GetHMFromRtc
 * ���������� ��RTC�ж�����ǰ��Сʱ�ͷ���
 * ��  �룺
 * ��  ���� psTime -- ��ǰʱ��ṹ
 ************************************************************************************************************/
void GetHMFromRtc(_tsTimeCb * psTime)
{
  UINT8 au8Temp[3];

  //��RTC���ԭʼ����
  RtcRead(CMD_HOUR, &au8Temp[0], 2);
  psTime->Hour   = RtcBcd2Hex( ByteExchange(au8Temp[0]) & 0x3f);
  psTime->Minute = RtcBcd2Hex( ByteExchange(au8Temp[1]) & 0x7f);
}

/*************************************************************************************************************
 * �������ƣ� GetMinuteFromRtc
 * ���������� ��RTC�ж�����ǰ�ķ�ʱ��
 * ��  �룺
 * ��  ���� ��ǰ�ķ�ʱ��
 ************************************************************************************************************/
UINT8 GetMinuteFromRtc(void)
{
  UINT8 au8Temp[8];
  UINT8 u8Minute;

  //��RTC���ԭʼ����
  RtcRead(CMD_YEAR, &au8Temp[0], 7);
  u8Minute = ByteExchange(au8Temp[5]) & 0x7f;

  return(RtcBcd2Hex(u8Minute));
}

/*************************************************************************************************************
 * �������ƣ� SetTimeToRtc
 * ���������� ��ʱ�����õ�RTC
 * ��  �룺 psTime -- ��ǰʱ��ṹ
 * ��  ����
 ************************************************************************************************************/
void SetTimeToRtc(_tsTimeCb * psTime)
{
  UINT8 au8Temp[8];

  au8Temp[0] = ByteExchange( RtcHex2Bcd(psTime->Year)  & 0xff);
  au8Temp[1] = ByteExchange( RtcHex2Bcd(psTime->Month) & 0x1f);
  au8Temp[2] = ByteExchange( RtcHex2Bcd(psTime->Date)  & 0x3f);

  psTime->Week = CaculateWeek(psTime->Year, psTime->Month, psTime->Date);
  au8Temp[3] = ByteExchange(psTime->Week & 0x07);

  au8Temp[4] = ByteExchange( RtcHex2Bcd(psTime->Hour)   & 0x7f);
  au8Temp[5] = ByteExchange( RtcHex2Bcd(psTime->Minute) & 0x7f);
  au8Temp[6] = ByteExchange( RtcHex2Bcd(psTime->Second) & 0x7f);
  au8Temp[7] = 0;

  RtcWrite(CMD_YEAR, &au8Temp[0], sizeof(_tsTimeCb));
  Delay(10);
}

