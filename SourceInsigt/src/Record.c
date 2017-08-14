/******************************************************************************
*											
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved										
*
* 文 件 名： Record.C
* 描    述： 记录

* 创建日期： 2014年9月5日11:31:59
* 作    者： Bob
* 当前版本： V1.00

------------------------------------------------------
*******************************************************************************/
#include "msp430x54x.h"
#include "DataType.h"
#include "global.h"
#include "string.h"
#include "ctype.h"
#include "I2C_Driver.h"
#include "Record.h"
#include "SystemCtl.h"


/********************************************************************************
 * 函数名称：ReadDebugSwtich
 * 函数功能：读调试开关
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void ReadDebugSwtich(void)
{
   g_bDebug = I2CMasterReadOneByte(EE_WRITE_ADDR, DEBUG_EN_ADDR);
}

/********************************************************************************
 * 函数名称：WriteDebugSwtich
 * 函数功能：写调试信息接口
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
 void SaveSystemNbr(void)
 {
   I2CMasterWriteOneByte(EE_WRITE_ADDR, );
 }

/********************************************************************************
 * 函数名称：RecordInit
 * 函数功能：信息初始化
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void RecordInit(void)
{
  UINT8 u8Temp;
  
  u8Temp = I2CMasterReadOneByte(EE_READ_ADDR, SYS_CFG_ADDR);
  if(0xff == u8Temp)
  {
    sRecordCb.
  }
  //读取北斗接收机号码
  ReadBdReceivingNbr();
  
  //读取CDMA接收机号码
  ReadCdmaReceivingNbr();
 
}

/********************************************************************************
 * 函数名称：RecordRun
 * 函数功能：接收串口发送的配置数据，并显示在显示器上
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void RecordRun(void)
{
  if((g_u8NeedSaved & SAVE_ALL_CFG) != SAVE_NONE)
  {
    //保存系统编号
    if(g_u8NeedSaved & SAVE_SYS_NBR)
    {
      SaveSystemNbr();
    }
    
    //保存设备类型。1 -- 围油栏，2 -- 收油机
    if(g_u8NeedSaved & SAVE_SYS_TYPE)
    {
      SaveSystemType();
    }
  
    //保存北斗接收机号码
    if(g_u8NeedSaved & SAVE_REV_BD_NBR)
    {
       SaveBdReceivingNbr();
    }
    
    //保存短信接收机号码
    if(g_u8NeedSaved & SAVE_REV_CDMA_NBR)
    { 
     SaveCdmaReceivingNbr();
    }
   
    g_u8NeedSaved = SAVE_NONE;
  }
}