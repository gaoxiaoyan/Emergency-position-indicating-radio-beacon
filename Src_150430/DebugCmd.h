
/******************************************************************************
*											
* Copyright (C) 2015, Sunic-ocean
* All Rights Reserved										
*
* 文 件 名： DebugCmd.h
* 描       述：

* 创建日期： 2015年4月29日16:41:13
* 作    者： Bob
* 当前版本： V1.00

------------------------------------------------------
*******************************************************************************/
#ifndef __DEBUG_CMD_H__
#define __DEBUG_CMD_H__

#include "DataType.h"

#ifdef    DEBUG_GLOBALS
#define   DEBUG_EXT
#else
#define   DEBUG_EXT   extern
#endif

#define CMD_HAND_SHAKE            0x00
#define CMD_GET_VERSION           0x01
#define CMD_ALL_INFO_PRINT        0x02
#define CMD_SET_RTC_TIME          0X03
#define CMD_GET_RTC_TIME          0x04
#define CMD_GPS_INFO_CLOSE        0x10
#define CMD_GPS_INFO_OPEN         0x11
#define CMD_BD_INFO_CLOSE         0x20
#define CMD_BD_INFO_OPEN          0x21
#define CMD_BD_INFO_TEST          0x22
#define CMD_CDMA_INFO_CLOSE       0x30
#define CMD_CDMA_INFO_OPEN        0x31
#define CMD_CDMA_GET_TIME         0x32
#define CMD_CDMA_SET_LOC_NUM      0x33
#define CMD_CDMA_GET_LOC_NUM      0x34
#define CMD_CDMA_SEND_NUM         0x35
#define CMD_CDMA_MODULE_NUM       0x36
#define CMD_CDMA_GET_CSQ          0x37
#define CMD_SEND_TIME             0x40
#define CMD_GET_BAT               0x50

/**********************************************************************************************
 函数名称：DebugCmdInit
 函数功能：Debug cmd parse init

 参数： 无
 返回： 无
**********************************************************************************************/
extern void DebugCmdInit(void);

/***************************************************************************
//函数名称：UartCmdRun
//功能描述：串口接收命令解析
//
//参数：无
//返回：无
***************************************************************************/
extern void DebugCmdRun(void);


#endif