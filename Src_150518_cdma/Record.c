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
#define  RECORD_GLOBAL
#include "msp430x54x.h"
#include "DataType.h"
#include "global.h"
#include "string.h"
#include "ctype.h"
#include "I2C_Driver.h"
#include "Record.h"
#include "SystemCtl.h"


/********************************************************************************
 * 函数名称：ReadSystemNbr
 * 函数功能：读取设备的编码
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void ReadSystemNbr(UINT16 *ptr)
{
	UINT8 u8Temp;

	u8Temp = I2CMasterReadOneByte(EE_READ_ADDR, SYS_NBR_ADDR+1);
	*ptr = (UINT16)u8Temp << 8;

	u8Temp = I2CMasterReadOneByte(EE_READ_ADDR, SYS_NBR_ADDR);
	*ptr += u8Temp;
}

/********************************************************************************
 * 函数名称：ReadSystemType
 * 函数功能：读取设备的类型
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void ReadSystemType(UINT8 *ptr)
{
	*ptr = I2CMasterReadOneByte(EE_READ_ADDR, SYS_TYPE_ADDR);
}

/********************************************************************************
 * 函数名称：ReadSystemGapTime
 * 函数功能：读取设备的信息发送间隔
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void ReadSystemGapTime(UINT8 *ptr)
{
	*ptr = I2CMasterReadOneByte(EE_READ_ADDR, SYS_GAP_ADDR);
}

/********************************************************************************
 * 函数名称：ReadBdReceivingNbr
 * 函数功能：读取北斗接收机号码
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void ReadBdReceivingNbr(UINT8 *ptr)
{
	ptr[0] = I2CMasterReadOneByte(EE_READ_ADDR, SYS_REV_BD_ADDR  );
	ptr[1] = I2CMasterReadOneByte(EE_READ_ADDR, SYS_REV_BD_ADDR+1);
	ptr[2] = I2CMasterReadOneByte(EE_READ_ADDR, SYS_REV_BD_ADDR+2);
}

/********************************************************************************
 * 函数名称：ReadBckBdReceivingNbr
 * 函数功能：读取备用北斗接收机号码
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void ReadBckBdReceivingNbr(UINT8 *ptr)
{
	ptr[0] = I2CMasterReadOneByte(EE_READ_ADDR, SYS_BCK_BD_ADDR  );
	ptr[1] = I2CMasterReadOneByte(EE_READ_ADDR, SYS_BCK_BD_ADDR+1);
	ptr[2] = I2CMasterReadOneByte(EE_READ_ADDR, SYS_BCK_BD_ADDR+2);
}

/********************************************************************************
 * 函数名称：ReadCdmaReceivingNbr
 * 函数功能：读取CDMA接收机号码
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void ReadCdmaReceivingNbr(UINT8 *ptr)
{
	I2CMasterRead(EE_READ_ADDR, SYS_REV_CDMA_ADDR, ptr, 11);
}

/********************************************************************************
 * 函数名称：ReadBckCdmaReceivingNbr
 * 函数功能：读取备用CDMA接收机号码
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void ReadBckCdmaReceivingNbr(UINT8 *ptr)
{
  I2CMasterRead(EE_READ_ADDR, SYS_BCK_CDMA_ADDR, ptr, 11);
}

/********************************************************************************
 * 函数名称：ReadSystemRunTime
 * 函数功能：读取程序上次运行时间
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void ReadSystemRunTime(UINT32 *ptr)
{
  UINT8 au8Buf[4];
  
  I2CMasterRead(EE_READ_ADDR, SYS_RUN_TIME_ADDR, &au8Buf[0], 4);
  
  *ptr  = ((UINT32)au8Buf[0] << 24);
  *ptr += ((UINT32)au8Buf[1] << 16);
  *ptr += ((UINT32)au8Buf[2] << 8);
  *ptr += au8Buf[3];
}

/********************************************************************************
 * 函数名称：SaveSystemNbr
 * 函数功能：保存系统编号
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
 void SaveSystemNbr(UINT16 *ptr)
 {
 	UINT8 *pBuf;

	pBuf = (UINT8 *)ptr;
  I2CMasterWrite(EE_WRITE_ADDR, SYS_NBR_ADDR, &pBuf[0], 2);
 }

/********************************************************************************
 * 函数名称：SaveSystemNbr
 * 函数功能：保存设备类型
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
 void SaveSystemType(UINT8 *ptr)
 {
 	UINT8 *pBuf;

	pBuf = ptr;
  I2CMasterWrite(EE_WRITE_ADDR,SYS_TYPE_ADDR, pBuf, 1);
 }

/********************************************************************************
 * 函数名称：SaveSystemGapTime
 * 函数功能：保存发送时间间隔
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
 void SaveSystemGapTime(UINT8 *ptr)
 {
 	UINT8 *pBuf;

	pBuf = ptr;
  I2CMasterWrite(EE_WRITE_ADDR,SYS_GAP_ADDR, pBuf, 1);
 }

/********************************************************************************
 * 函数名称：SaveBdReceivingNbr
 * 函数功能：保存北斗接收机号码
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void SaveBdReceivingNbr(UINT8 *ptr)
{
	I2CMasterWrite(EE_WRITE_ADDR,SYS_REV_BD_ADDR, ptr, 3);
}

 /********************************************************************************
 * 函数名称：SaveBckBdReceivingNbr
 * 函数功能：保存备用北斗接收机号码
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void SaveBckBdReceivingNbr(UINT8 *ptr)
{
	I2CMasterWrite(EE_WRITE_ADDR, SYS_BCK_BD_ADDR, ptr, 3);
}

/********************************************************************************
 * 函数名称：SaveBdReceivingNbr
 * 函数功能：保存cdma接收机号码
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void SaveCdmaReceivingNbr(UINT8 *ptr)
{
	I2CMasterWrite(EE_WRITE_ADDR,SYS_REV_CDMA_ADDR, ptr, 11);
}

/********************************************************************************
 * 函数名称：SaveBckCdmaReceivingNbr
 * 函数功能：备用cdma接收机号码
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void SaveBckCdmaReceivingNbr(UINT8 *ptr)
{
	I2CMasterWrite(EE_WRITE_ADDR,SYS_BCK_CDMA_ADDR, ptr, 11);
}

/********************************************************************************
 * 函数名称：SaveSystemRunTime
 * 函数功能：保存程序运行时间
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void SaveSystemRunTime(UINT32 *ptr)
{
  UINT8 au8Buf[4];
  
  au8Buf[0] = (UINT8)(*ptr >> 24);
  au8Buf[1] = (UINT8)(*ptr >> 16);
  au8Buf[2] = (UINT8)(*ptr >>  8);
  au8Buf[3] = (UINT8)(*ptr);
  
  I2CMasterWrite(EE_WRITE_ADDR,SYS_RUN_TIME_ADDR, &au8Buf[0], 4);
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
    sRecordCb.u16SysNbr      = 0;
		sRecordCb.u8SysType      = SYS_TYPE_WYL;
    sRecordCb.u8GapTime      = 5;

		memset(sRecordCb.au8BdRevNbr, 0, 3);
		memset(sRecordCb.au8BdBckNbr, 0, 3);

		memset(sRecordCb.au8CdmaRevNbr, 0, 11);
		memset(sRecordCb.au8CdmaBckNbr, 0, 11);
    
    sRecordCb.u32SysRunTime = 0;
  }
	else
	{ 
    //读取设备的编号
    ReadSystemNbr(&sRecordCb.u16SysNbr);  //注意大小端模式
    
    //读取设备的类型
    ReadSystemType(&sRecordCb.u8SysType);
    
    //时间间隔
    ReadSystemGapTime(&sRecordCb.u8GapTime);
    
	  //读取北斗接收机号码
	  ReadBdReceivingNbr(&sRecordCb.au8BdRevNbr[0]);
	  
	  //读取CDMA接收机号码
	  ReadCdmaReceivingNbr(&sRecordCb.au8CdmaRevNbr[0]);
    
    ReadSystemRunTime(&sRecordCb.u32SysRunTime);
	}
 
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
      SaveSystemNbr(&sRecordCb.u16SysNbr);
    }
    
    //保存设备类型。1 -- 围油栏，2 -- 收油机
    if(g_u8NeedSaved & SAVE_SYS_TYPE)
    {
      SaveSystemType(&sRecordCb.u8SysType);
    }
    
    //保存发送时间间隔
    if(g_u8NeedSaved & SAVE_SYS_GAP)
    {
      SaveSystemGapTime(&sRecordCb.u8GapTime);
    }
  
    //保存北斗接收机号码
    if(g_u8NeedSaved & SAVE_REV_BD_NBR)
    {
       SaveBdReceivingNbr(&sRecordCb.au8BdRevNbr[0]);
    }
    
    //保存短信接收机号码
    if(g_u8NeedSaved & SAVE_REV_CDMA_NBR)
    { 
      SaveCdmaReceivingNbr(&sRecordCb.au8CdmaRevNbr[0]);
    }
   
    g_u8NeedSaved = SAVE_NONE;
  }
}




