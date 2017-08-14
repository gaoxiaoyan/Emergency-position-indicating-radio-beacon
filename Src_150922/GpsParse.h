

/*
  GPS 指令解析头文件
*/

#ifndef __GPS_PARSE_H__
#define __GPS_PARSE_H__

#include "msp430x54x.h"

#include "global.h"
#include "Uart2.h"
#include "string.h"

#ifdef    GPS_GLOBALS
#define   GPS_EXT
#else
#define   GPS_EXT   extern
#endif

GPS_EXT BOOL g_bGpsCompleted;
//第一次上电使用接收标志
GPS_EXT BOOL  g_bGpsFirstRx;
GPS_EXT SINT8 g_s8GpsFirstDog;

/***********************************************************************************
函数名称：Uart0CmdInit()
函数功能：GPS数据组包结构初始化

参数：    无
返回：    无
***********************************************************************************/
extern void GpsCmdInit(void);

/**********************************************************************************************
 函数名称：GpsCmdRun
 函数功能：GPS 命令解析，连续采集8组有效数据，找出卫星质量好的一组数据

 参数： 无
 返回： 无
**********************************************************************************************/
extern void GpsCmdRun(void);


#endif



