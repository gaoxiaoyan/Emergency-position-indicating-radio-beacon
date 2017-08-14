

#ifndef __DTUMODULE_H__
#define __DTUMODULE_H__

#include "DataType.h"

//CDMA_RI中断接口
#define CdmaRi                BIT7
#define CdmaRiDir             P1DIR
#define CdmaRiIn              P1IN
#define CdmaRiIntFlag         P1IFG 
#define CdmaRiIES             P1IES
#define CdmaRiIE              P1IE
#define CdmaRiIntCheck        (CdmaRiIntFlag & CdmaRi)
#define CdmaRiIntEnable()     (CdmaRiIE | CdmaRi)
#define CdmaRiIntDisable()    (CdmaRiIE & (~CdmaRi))
#define CdmaRiIesEdge()       (CdmaRiIES | CdmaRi)

//CDMA电源控制
#define CdmaPower              BIT1
#define CdmaPowerOut           P10OUT
#define CdmaPowerDir           P10DIR
#define CdmaPowerOn()          CdmaPowerOut |=  CdmaPower
#define CdmaPowerOff()         CdmaPowerOut &= ~CdmaPower

//CDMA ON/OF
#define CdmaOnOff              BIT0
#define CdmaOnOffOut           P3OUT
#define CdmaOnOffDir           P3DIR
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

/*****************************************************************************
 函 数 名  : AssembleCdmaMsg
 功能描述  : AssembleCdmaMsg

 输入参数  : 发送信息指针
 返 回 值  : none,
*****************************************************************************/
extern void AssembleCdmaMsg(UINT8 *pBuf, UINT8 u8Len);

/*****************************************************************************
 函 数 名  : GetCdmaCSQ
 功能描述  : 获取CDMA信号强度

 输入参数  : none
 返 回 值  : 信号强度,0~31
*****************************************************************************/
extern UINT8 GetCdmaCSQ(void);

/*****************************************************************************
 函 数 名  : GetCdmaTime
 功能描述  : 获取CDMA时间

 输入参数  : none
 返 回 值  : none
*****************************************************************************/
extern BOOL SetCdmaTimeToRtc(void);


#endif