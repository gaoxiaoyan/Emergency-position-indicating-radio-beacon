

#ifndef __SYSTEMCTL_H__
#define __SYSTEMCTL_H__

#ifdef SYS_GLOBAL
#define SYS_EXT
#else
#define SYS_EXT extern
#endif

//待机模式标志位
SYS_EXT BOOL g_bStandby;
SYS_EXT UINT8 g_bDebug;

//cdma发送使能标志
SYS_EXT BOOL g_bCdmaSend;
//北斗发送使能标志
SYS_EXT BOOL g_bBdSend;
//准点发送使能标志
SYS_EXT BOOL g_bSendInfo;
//提前一分钟唤醒通讯模块标志能使
SYS_EXT BOOL g_bCommuWake;
//待机中CDMA的短信RI标志
SYS_EXT BOOL g_bCdmaRiEvent;

/*****************************************************************************
 *函数名称: InitClk
 *功能描述: 系统时钟初始化

 *参数: 系统启动写1，唤醒启动写0，主要针对延时
 *返回:
*****************************************************************************/
extern void InitClk(BOOL bFlag);

/******************系统初始化******************/
extern void SystemInit(void);

/*****************************************************************************
 函 数 名  : WaitSystemStart
 功能描述  : 系统开机控制

  参  数  : NONE
 返 回 值 : NONE
*****************************************************************************/
//extern void WaitSystemStart(void);

extern void SystemIdleInit(void);
extern void SystemIdleRun(void);

//把主时钟和子时钟由DCO切换到XT1
extern void DcoSwitchXt1(void);

/****************************************************************
 * 函数名称：MemClr
 * 功    能：内存设置为0
 *
 * 参    数：UINT8 *pu8Buf,  UINT8 u8Len
 * 返回值  ：无
****************************************************************/
extern void MemClr(UINT8 *pu8Buf,  UINT8 u8Len);

/****************************************************************
 * 函数名称：MEM_CPY
 * 功    能：将BUF2中内容复制到BUF1中
 *
 * 参    数：UINT8 *pu8Buf1, UINT8 *pu8Buf2, UINT8 u8Len
 * 返回值  ：无
****************************************************************/
extern void MEM_CPY(void *pu8Buf1, const void *pu8Buf2, UINT8 u8Len);

/****************************************************************
 * 函数名称：MEM_CMP
 * 功    能：内存比较。
 *
 * 参    数：const void *cs, const void *ct, UINT8 count
 * 返回值  ：SINT8
****************************************************************/
extern SINT8 MEM_CMP(const void *cs, const void *ct, UINT8 count);

/***************************************************************************
//函数名称：PrintfCmdList
//功能描述：打印命令列表
//
//参数：无
//返回：无
***************************************************************************/
extern void PrintfCmdList(void);

/***************************************************************************
//函数名称：EnterLowPower
//功能描述：数据发送成功，进入低功耗模块
//
//参数：无
//返回：无
***************************************************************************/
extern void EnterLowPower(void);

/***************************************************************************
//函数名称：SystemIdleInit
//功能描述：初始化
//
//参数：无
//返回：无
***************************************************************************/
extern void SystemIdleInit(void);

/***************************************************************************
//函数名称：SytemIdleRun
//功能描述：程序运行指示灯
//
//参数：无
//返回：无
***************************************************************************/
extern void SystemIdleRun(void);


#endif