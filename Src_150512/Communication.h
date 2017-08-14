
#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include "DataType.h"

#ifdef  COMMU_GLOBAL
#define COM_EXT
#else
#define COM_EXT extern 
#endif

typedef struct
{
  UINT8 u8Index;    //本机编号
  UINT8 *pGPS_Y;    //gps纬度
  UINT8 *pGPS_X;    // gps经度
  UINT8 u8Battery;  //电池电量
  UINT8 u8GapTime;  //发送间隔时间1~6代表5~30分钟，步进5分钟
}_tsSendInfo;

COM_EXT _tsSendInfo sSendInfo;

/*****************************************************************************
 函 数 名  : CommunicationInit
 功能描述  : 发送数据结构初始化

 输入参数  : none
 返 回 值  : none,
*****************************************************************************/
extern void CommunicationInit(void);

/*****************************************************************************
 函 数 名  : SendInfoToCenter
 功能描述  : 发送数据到中心，

 输入参数  : none
 返 回 值  : none,
*****************************************************************************/
extern void SendInfoToCenter(void);

/****************************************************************
 * 函数名称：ComModuleSwitch
 * 功    能：判断CDMA模块信号强度，若满足发射要求则使用CDMA发送；
             否则打开北斗模块
 *
 * 参    数：无
 * 返回值  ：无
****************************************************************/
extern void ComModuleSwitch(void);

#endif