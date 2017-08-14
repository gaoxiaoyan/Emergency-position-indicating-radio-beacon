

#ifndef __RTC_H__
#define __RTC_H__

#ifdef  RTC_GLOBAL
#define 

//�ڲ��Ĵ���
#define CMD_STAT_1         (0 << 1)
#define CMD_STAT_2         (1 << 1)
#define CMD_YEAR           (2 << 1)
#define CMD_HOUR           (3 << 1)
#define CMD_ALARM_1        (4 << 1)
#define CMD_ALARM_2        (5 << 1)
#define CMD_CALIBRATION    (6 << 1)

//����A��ʹ��λ
#define A1WE              BIT0
#define A1HE              BIT0
#define A1ME              BIT0

//��ȡ״̬�Ĵ���1
#define STAT1_RESET       BIT7
#define STAT1_CONVER      BIT6
#define STAT1_SC0         BIT5
#define STAT1_SC1         BIT4
#define STAT1_INT1        BIT3
#define STAT1_INT2        BIT2
#define STAT1_BLD         BIT1
#define STAT1_POC         BIT0

//��ȡ״̬�Ĵ���2
#define STAT2_INT1FE      BIT7
#define STAT2_INT1ME      BIT6
#define STAT2_INT1AE      BIT5
#define STAT2_INT1_32K    BIT4
#define STAT2_INT2FE      BIT3
#define STAT2_INT2ME      BIT2
#define STAT2_INT2AE      BIT1
#define STAT2_TEST        BIT0

//��ǰʱ��ṹ����
typedef struct
{
  UINT8 Year;
  UINT8 Month;
  UINT8 Date;
  UINT8 Week;
  UINT8 Hour;
  UINT8 Minute;
  UINT8 Second;
}_tsTimeCb;


/*************************************************************************************************************
 * �������ƣ� RtcInit
 * ���������� RTC��ʼ��
 * ��  �룺 psTime -- ��ǰʱ��ṹ
 * ��  ���� 					
 ************************************************************************************************************/
extern void RtcInit(void);

/*************************************************************************************************************
 * �������ƣ� GetTimeFromRtc
 * ���������� ��RTC�ж�����ǰ�����ں�ʱ��
 * ��  �룺 psTime -- ��ǰʱ��ṹ
 * ��  ���� 					
 ************************************************************************************************************/
extern void GetTimeFromRtc(_tsTimeCb * sTime);

/*************************************************************************************************************
 * �������ƣ� SetTimeToRtc
 * ���������� ��ʱ�����õ�RTC
 * ��  �룺 psTime -- ��ǰʱ��ṹ
 * ��  ���� 					
 ************************************************************************************************************/
extern void SetTimeToRtc(_tsTimeCb * psTime);

/*************************************************************************************************************
 * �������ƣ� RtcConverHour
 * ���������� 12/24��������
 * ��  �룺 bConver 1-- 24���ƣ� 0 -- 12����
 * ��  ���� 					
 ************************************************************************************************************/
extern void RtcConverHour(BOOL bConver);

//��RTC�л�ȡʱ���
extern void GetStampFromRtc(void);

/*************************************************************************************************************
 * �������ƣ� GetMinuteFromRtc
 * ���������� ��RTC�ж�����ǰ�ķ�ʱ��
 * ��  �룺
 * ��  ���� ��ǰ�ķ�ʱ��				
 ************************************************************************************************************/
extern UINT8 GetMinuteFromRtc(void);

#endif