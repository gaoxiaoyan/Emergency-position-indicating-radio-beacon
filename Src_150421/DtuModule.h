

#ifndef __DTUMODULE_H__
#define __DTUMODULE_H__

#include "DataType.h"

//DTU配置返回状态
#define DTU_CFG_MODE          0u
#define DTU_CFG_OK            1u
#define DTU_CFG_ERROR         2u
#define DTU_UART_SET_ERROR    3u
#define DTU_TCP_SET_ERROR     4u
#define DTU_AUTO_SET_ERROR    5u
#define DTU_ID_SET_ERROR      6u
#define DTU_AGAIN_SET_ERROR   7u
#define DTU_IDLE_SET_ERROR    8u
#define DTU_BEAT_SET_ERROR    9u
#define DTU_GAP_SET_ERROR     10u
#define DTU_MUL_SET_ERROR     11u
#define DTU_DCS_SET_ERROR     12u
#define DTU_MODE_SET_ERROR    13u
#define DTU_DEB_SET_ERROR     14u
#define DTU_IP_SUB1_ERROR     15u
#define DTU_IP_SUB2_ERROR     15u
#define DTU_IP_SUB3_ERROR     15u
#define DTU_IP_SUB4_ERROR     15u

/******************************************************************************
 * 函数名称：GetDtuSvr
 * 描述：获取DTU的上线状态
 *
 * 参数：无
 * 返回：bFlag； 1 -- 上线。0 -- 没有上线
 *****************************************************************************/
extern BOOL GetDtuSvr(void);

/******************************************************************************
 * 函数名称：DtuInfoSend
 * 描述：发送信息到中心。
 *
 * 参数：无
 * 返回：无
 *****************************************************************************/
extern void DtuInfoSend(void);

/******************************************************************************
 * 函数名称：DtuCmdInit
 * 描述：DTU命令解析初始化
 *
 * 参数：无
 * 返回：无
 *****************************************************************************/
extern void DtuCmdInit(void);

/******************************************************************************
 * 函数名称：DtuCmdRun
 * 描述：DTU命令解析
 *
 * 参数：无
 * 返回：无
 *****************************************************************************/
extern void DtuCmdRun(void);

/*****************************************************************************
 函 数 名  : ConfigDtuModule
 功能描述  : Config dtu module, 

 输入参数  : UINT8 u8State
 返 回 值  :
*****************************************************************************/
extern void ConfigDtuModule(UINT8 u8State);

/*****************************************************************************
 函 数 名  : ConfigDtuModule
 功能描述  : Config dtu module, 

 输入参数  : UINT8 u8State
 返 回 值  :
*****************************************************************************/
extern UINT8 McuConfigDtuMainServer(void);

/*****************************************************************************
 函 数 名  : McuConfigDtuSub1
 功能描述  : Config dtu module, 

 输入参数  : UINT8 u8State
 返 回 值  :
*****************************************************************************/
extern UINT8 McuConfigDtuSub1(UINT8 u8CfgEnable);

/*****************************************************************************
 函 数 名  : McuConfigDtuSub2
 功能描述  : Config dtu module, 

 输入参数  : UINT8 u8State
 返 回 值  :
*****************************************************************************/
extern UINT8 McuConfigDtuSub2(UINT8 u8CfgEnable);

/*****************************************************************************
 函 数 名  : McuConfigDtuSub3
 功能描述  : Config dtu module, 

 输入参数  : UINT8 u8State
 返 回 值  :
*****************************************************************************/
extern UINT8 McuConfigDtuSub3(UINT8 u8CfgEnable);

/*****************************************************************************
 函 数 名  : McuConfigDtuSub4
 功能描述  : Config dtu module, 

 输入参数  : UINT8 u8State
 返 回 值  :
*****************************************************************************/
extern UINT8 McuConfigDtuSub4(UINT8 u8CfgEnable);

/*****************************************************************************
 函 数 名  : GetDtuConfigInfo
 功能描述  : Get DTU module config, Here mainly get module ID.

 输入参数  : none
 返 回 值  : 配置状态
*****************************************************************************/
extern UINT8 GetDtuConfigInfo(UINT8 *pu8Buf);

#endif