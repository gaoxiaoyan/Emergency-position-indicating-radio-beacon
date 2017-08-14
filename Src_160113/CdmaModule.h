

#ifndef __DTUMODULE_H__
#define __DTUMODULE_H__

#include "global.h"

//CDMA模块最多接收短信数
#define  MSG_MAX_SIZE       30

//CDMA_RI中断接口
#define CdmaRi                BIT7
#define CdmaRiDir             P1DIR
#define CdmaRiOut             P1OUT
#define CdmaRiIn              P1IN
#define CdmaRiIntFlag         P1IFG
#define CdmaRiIES             P1IES
#define CdmaRiIE              P1IE
#define CdmaRiIntCheck()      ((CdmaRiIntFlag & CdmaRi) == CdmaRi)
#define CdmaRiIntFlagClr()    (CdmaRiIntFlag &= ~CdmaRi)
#define CdmaRiIntEnable()     (CdmaRiIE  |=  CdmaRi)
#define CdmaRiIntDisable()    (CdmaRiIE  &= ~CdmaRi)
#define CdmaRiIesEdge()       (CdmaRiIES |=  CdmaRi)

#define CdmaDtr               BIT4
#define CdmaDtrDir            P4DIR
#define CdmaDtrOut            P4OUT
#define CdmaDtrSetOut()       (CdmaDtrDir |=  CdmaDtr)
#define CdmaDtrSetHigh()      (CdmaDtrOut |=  CdmaDtr)
#define CdmaDtrSetLow()       (CdmaDtrOut &= ~CdmaDtr)

//CDMA电源控制
#define CdmaPower              BIT4
#define CdmaPowerOut           P1OUT
#define CdmaPowerDir           P1DIR
#define CdmaPowerOn()          CdmaPowerOut |=  CdmaPower
#define CdmaPowerOff()         CdmaPowerOut &= ~CdmaPower

#if DTIC_DET_EN > 0
#define CdmaIcCheck            BIT7
#define CdmaIcCheckDir         P2DIR
#define CdmaIcCheckOut         P2OUT
#define CdmaIcCheckIn          P2IN
#define CdmaIcCheckSetIn()     (CdmaIcCheckDir &= ~CdmaIcCheck)
#define CdmaIcDet()            ((CdmaIcCheckIn & CdmaIcCheck) == CdmaIcCheck)
#endif

//CDMA ON/OF
#define CdmaOnOff              BIT0
#define CdmaOnOffOut           P4OUT
#define CdmaOnOffDir           P4DIR
#define SetCdmaOnOff(x)        (x)? (CdmaOnOffOut |=  CdmaOnOff) : (CdmaOnOffOut &= ~CdmaOnOff)

//CDMA reset复位
#define CdmaReset              BIT1
#define CdmaResetOut           P3OUT
#define CdmaResetDir           P3DIR
#define SetCdmaReset(x)        (x)? (CdmaResetOut |=  CdmaReset) : (CdmaResetOut &= ~CdmaReset)


/*****************************************************************************
 函 数 名  : CdmaInit
 功能描述  : CDMA模块初始化

 输入参数  : none
 返 回 值  : none,
*****************************************************************************/
extern void CdmaInit(void);

extern void CdmaToBeiDou(void);
extern void BeiDouToCdma(void);

/*****************************************************************************
 函 数 名  : AssembleCdmaMsg
 功能描述  : AssembleCdmaMsg

 输入参数  : *pNum -- 发送号码的指针
             *pBuf -- 发送信息指针

 返 回 值  : TRUE, FALSE
*****************************************************************************/
extern BOOL AssembleCdmaMsg(UINT8 *pNum, UINT8 *pBuf, UINT8 u8Len);

/*****************************************************************************
 函 数 名  : GetCdmaCSQ
 功能描述  : 获取CDMA信号强度

 输入参数  : none
 返 回 值  : 信号强度,0~31
*****************************************************************************/
extern UINT8 GetCdmaCSQ(void);

/*****************************************************************************
 函 数 名  : DeletCdmaMsg()
 功能描述  : 短信删除功能

 输入参数  : s8Index -- 所要删除的短信编号, 最多存储31条短信
                        31 -- 删除全部的短消息

 返 回 值  : TRUE, FALSE
*****************************************************************************/
extern BOOL DeletCdmaMsg(SINT8 s8Index);

/*****************************************************************************
 函 数 名  : GetCdmaTime
 功能描述  : 获取CDMA时间, 设置到RTC

 输入参数  : *pTime，*pu8Len
 返 回 值  : none
*****************************************************************************/
extern void GetCdmaTime(UINT8 *pTime, UINT8 *pu8Len);

/*****************************************************************************
 函 数 名  : GetCdmaTime
 功能描述  : 获取CDMA时间

 输入参数  : none
 返 回 值  : none
*****************************************************************************/
extern BOOL SetCdmaTimeToRtc(void);

/*****************************************************************************
 函 数 名  : SetLocationNbr
 功能描述  : 设置本机号码

 输入参数  : none
 返 回 值  : TRUE , FALSE
*****************************************************************************/
extern BOOL SetLocationNbr(UINT8 *prt);

/*****************************************************************************
 函 数 名  : GetLocationNbr
 功能描述  : 设置本机号码

 输入参数  : NONE
 返 回 值  : TRUE , FALSE
*****************************************************************************/
extern BOOL GetLocationNbr(UINT8 *ptr);

/*****************************************************************************
 函 数 名  : CdmaLowPowerEnter
 功能描述  : CDMA模块进入低功耗模块

 输入参数  : PTR
 返 回 值  : TRUE , FALSE
*****************************************************************************/
extern void CdmaLowPowerEnter(void);

/*****************************************************************************
 函 数 名  : CdmaLowPowerExit
 功能描述  : CDMA模块退出低功耗模块

 输入参数  : PTR
 返 回 值  : TRUE , FALSE
*****************************************************************************/
extern void CdmaLowPowerExit(void);

/*****************************************************************************
 函 数 名  : CdmaWakeUpCfg
 功能描述  : CDMA模块退出睡眠模式，并读取CDMA信号，信号强度低于16时自动切换到
             北斗通讯模块上

 输入参数  : PTR
 返 回 值  : TRUE , FALSE
*****************************************************************************/
extern void CdmaWakeUpCfg(void);

/*****************************************************************************
 函 数 名  : CdmaCmdRun
 功能描述  : CDMA数据解析

 输入参数  : 无
 返 回 值  : 无 , 无
*****************************************************************************/
extern void CdmaCmdRun(void);

#endif
