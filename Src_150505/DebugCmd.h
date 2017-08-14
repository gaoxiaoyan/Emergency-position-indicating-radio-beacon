
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

#define CMD_HAND_SHAKE            0x00    //握手命令，波特率115200
#define CMD_GET_VERSION           0x01    //获取设备版本信息
#define CMD_ALL_INFO_PRINT        0x02    //获取命令列表
#define CMD_SET_RTC_TIME          0X03    //设置RTC时间，从CDMA上读取
#define CMD_GET_RTC_TIME          0x04    //获取RTC时间。
#define CMD_GET_SYS_BAT           0x05    //获取系统剩余电量
#define CMD_SEND_GAP_TIME         0x06    //设置发送信息间隔
#define CMD_SET_SYS_INDEX         0x07    //设置设备编码
#define CMD_GET_SYS_INDEX         0x08    //设置设备编码
#define CMD_GPS_INFO_CLOSE        0x10    //GPS调试信息关
#define CMD_GPS_INFO_OPEN         0x11    //GPS调试信息开
#define CMD_BD_INFO_CLOSE         0x20    //北斗调试信息关
#define CMD_BD_INFO_OPEN          0x21    //北斗调试信息开
#define CMD_BD_INFO_TEST          0x22    //北斗发送报文测试
#define CMD_CDMA_INFO_CLOSE       0x30    //CDMA调试信息关
#define CMD_CDMA_INFO_OPEN        0x31    //CDMA调试信息开
#define CMD_CDMA_GET_TIME         0x32    //获取CDMA时间
#define CMD_CDMA_SET_LOC_NUM      0x33    //设置本机号码
#define CMD_CDMA_GET_LOC_NUM      0x34    //获取本机号码
#define CMD_CDMA_SET_SEND_NUM     0x35    //设置发送号码
#define CMD_CDMA_GET_SEND_NUM     0x36    //获取发送号码
#define CMD_CDMA_GET_CSQ          0x37    //获取信号强度
#define CMD_CDMA_INFO_TEST        0x38    //发送短信测试


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