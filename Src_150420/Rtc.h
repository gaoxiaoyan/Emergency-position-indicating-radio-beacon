

#ifndef __RTC_H__
#define __RTC_H__

//内部寄存器
#define CMD_STAT_1         (0 << 1)
#define CMD_STAT_2         (1 << 1)
#define CMD_YEAR           (2 << 1)
#define CMD_HOUR           (3 << 1)
#define CMD_ALARM_1        (4 << 1)
#define CMD_ALARM_2        (5 << 1)
#define CMD_CALIBRATION    (6 << 1)

//报警A的使能位
#define A1WE              BIT0
#define A1HE              BIT0
#define A1ME              BIT0

//存取状态寄存器1
#define STAT1_RESET       BIT7
#define STAT1_CONVER      BIT6
#define STAT1_SC0         BIT5
#define STAT1_SC1         BIT4
#define STAT1_INT1        BIT3
#define STAT1_INT2        BIT2
#define STAT1_BLD         BIT1
#define STAT1_POC         BIT0

//存取状态寄存器2
#define STAT2_INT1FE      BIT7
#define STAT2_INT1ME      BIT6
#define STAT2_INT1AE      BIT5
#define STAT2_INT1_32K    BIT4
#define STAT2_INT2FE      BIT3
#define STAT2_INT2ME      BIT2
#define STAT2_INT2AE      BIT1
#define STAT2_TEST        BIT0


/*************************************************************************************************************
 * 函数名称： RtcInit
 * 功能描述： RTC初始化
 * 输  入： psTime -- 当前时间结构
 * 输  出： 					
 ************************************************************************************************************/
extern void RtcInit(void);

/*************************************************************************************************************
 * 函数名称： GetTimeFromRtc
 * 功能描述： 从RTC中读出当前的日期和时间
 * 输  入： psTime -- 当前时间结构
 * 输  出： 					
 ************************************************************************************************************/
extern void GetTimeFromRtc(_tsTimeCb * sTime);

/*************************************************************************************************************
 * 函数名称： SetTimeToRtc
 * 功能描述： 将时间设置到RTC
 * 输  入： psTime -- 当前时间结构
 * 输  出： 					
 ************************************************************************************************************/
extern void SetTimeToRtc(_tsTimeCb * psTime);

/*************************************************************************************************************
 * 函数名称： RtcConverHour
 * 功能描述： 12/24进制设置
 * 输  入： bConver 1-- 24进制； 0 -- 12进制
 * 输  出： 					
 ************************************************************************************************************/
extern void RtcConverHour(BOOL bConver);

//从RTC中获取时间戳
extern void GetStampFromRtc(void);
/*************************************************************************************************************
 * 函数名称： GetMinuteFromRtc
 * 功能描述： 从RTC中读出当前的分时间
 * 输  入：
 * 输  出： 当前的分时间				
 ************************************************************************************************************/
extern UINT8 GetMinuteFromRtc(void);

#endif