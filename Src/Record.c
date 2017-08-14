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
#include "Communication.h"


/********************************************************************************
 * 函数名称：ReadCompanyNbr
 * 函数功能：读取设备的公司编码
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void ReadCompanyNbr(UINT16 *ptr)
{
    UINT8 u8Temp=0;

    u8Temp = I2CMasterReadOneByte(EE_READ_ADDR, SYS_CMP_ADDR+1);
	*ptr = (UINT16)u8Temp << 8;

	u8Temp = I2CMasterReadOneByte(EE_READ_ADDR, SYS_CMP_ADDR);
	*ptr += u8Temp;
}

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
	UINT8 u8Temp=0;

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
    if((*ptr > 99) || (*ptr < 1))
    {
        *ptr = SYS_TYPE_WYL;
    }
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
    //若间隔时间超范围，则设置为默认
    if((*ptr < '1') && (*ptr > '6'))
    {
        *ptr = '1';
    }
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

void ReadSystemBdIndex(UINT8 *ptr, UINT8 *pSource)
{
    UINT32 u32Addr=0;
    UINT8  i = 0;

    u32Addr  = ((UINT32)pSource[0]) << 16;
    u32Addr += ((UINT32)pSource[1]) << 8;
    u32Addr +=  (UINT32)pSource[2];

    ptr[i++] = '0';
    ptr[i++] = '0';
    ptr[i++] = '0';
    ptr[i++] = u32Addr / 10000000 + '0';
    ptr[i++] = u32Addr % 10000000/1000000 + '0';
    ptr[i++] = u32Addr % 1000000/100000 + '0';
    ptr[i++] = u32Addr % 100000/10000 + '0';
    ptr[i++] = u32Addr % 10000/1000 + '0';
    ptr[i++] = u32Addr % 1000/100 + '0';
    ptr[i++] = u32Addr % 100/10 + '0';
    ptr[i++] = u32Addr % 10 + '0';
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
 * 函数名称：ReadCommunicationMode
 * 函数功能：读取通讯方式 1 -- CDMA; 2 -- BD; 3 -- CDMA + BD自动
 *
 * 输入： 无
 * 返回： UINT8 -- 通讯模式
 *
 *******************************************************************************/
UINT8 ReadCommunicationMode(void)
{
    return(I2CMasterReadOneByte(EE_READ_ADDR, SYS_COM_SEL_ADDR) & 0x03);
}

/********************************************************************************
 * 函数名称：SaveCommunicationMode
 * 函数功能：保存通讯方式 BIT0 -- CDMA; BIT1 -- BD; BIT0+BIT1 -- CDMA + BD自动
 *
 * 输入： UINT8 -- 通讯方式
 * 返回： 无
 *
 *******************************************************************************/
void SaveCommunicationMode(UINT8 u8Mode)
{
    I2CMasterWriteOneByte(EE_WRITE_ADDR, SYS_COM_SEL_ADDR, (u8Mode & 0x03));
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

void ReadSystemCdmaIndex(UINT8 *ptr, UINT8 *pSource)
{
    memcpy(ptr, pSource, 11);
}

void ReadCdmaLocationNbr(UINT8 *ptr)
{
    I2CMasterRead(EE_READ_ADDR, SYS_LOC_CDMA_ADDR, ptr, 11);
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
 * 函数名称：SaveCompanyNbr
 * 函数功能：保存厂家编号
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
 void SaveCompanyNbr(UINT16 *ptr)
 {
 	UINT8 *pBuf;

	pBuf = (UINT8 *)ptr;
    I2CMasterWrite(EE_WRITE_ADDR, SYS_CMP_ADDR, &pBuf[0], 2);
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

void SaveSystemBdIndex(UINT8 *ptr, UINT8 *pSource)
{
    UINT32 u32Addr=0;
    UINT8  i = 0;

    u32Addr  = ((UINT32)pSource[0]) << 16;
    u32Addr += ((UINT32)pSource[1]) << 8;
    u32Addr +=  (UINT32)pSource[2];

    ptr[i++] = '0';
    ptr[i++] = '0';
    ptr[i++] = '0';
    ptr[i++] = u32Addr / 10000000 + '0';
    ptr[i++] = u32Addr % 10000000/1000000 + '0';
    ptr[i++] = u32Addr % 1000000/100000 + '0';
    ptr[i++] = u32Addr % 100000/10000 + '0';
    ptr[i++] = u32Addr % 10000/1000 + '0';
    ptr[i++] = u32Addr % 1000/100 + '0';
    ptr[i++] = u32Addr % 100/10 + '0';
    ptr[i++] = u32Addr % 10 + '0';
}

/********************************************************************************
 * 函数名称：SaveBdBckReceivingNbr
 * 函数功能：保存北斗接收机号码
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void SaveBdBckReceivingNbr(UINT8 *ptr)
{
	I2CMasterWrite(EE_WRITE_ADDR,SYS_BCK_BD_ADDR, ptr, 3);
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

void SaveSystemCdmaIndex(UINT8 *ptr)
{
    I2CMasterWrite(EE_WRITE_ADDR,SYS_LOC_CDMA_ADDR, ptr, 11);
}

void SaveCdmaLocationNbr(UINT8 *ptr)
{
	I2CMasterWrite(EE_WRITE_ADDR,SYS_LOC_CDMA_ADDR, ptr, 11);
}

/********************************************************************************
 * 函数名称：SaveCdmaBckReceivingNbr
 * 函数功能：保存cdma备份接收机号码
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void SaveCdmaBckReceivingNbr(UINT8 *ptr)
{
	I2CMasterWrite(EE_WRITE_ADDR,SYS_BCK_CDMA_ADDR, ptr, 11);
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

  I2CMasterWrite(EE_WRITE_ADDR, SYS_RUN_TIME_ADDR, &au8Buf[0], 4);
}

#ifdef BEIDOU_TEST
void ReadBdRxNbr(UINT16 *ptr)
{
    UINT8 u8Temp;

	u8Temp = I2CMasterReadOneByte(EE_READ_ADDR, SYS_BD_RX_ADDR+1);
	*ptr = (UINT16)u8Temp << 8;

	u8Temp = I2CMasterReadOneByte(EE_READ_ADDR, SYS_BD_RX_ADDR);
	*ptr += u8Temp;
}

void ReadBdSendNbr(UINT16 *ptr)
{
    UINT8 u8Temp;

	u8Temp = I2CMasterReadOneByte(EE_READ_ADDR, SYS_BD_TX_ADDR+1);
	*ptr = (UINT16)u8Temp << 8;

	u8Temp = I2CMasterReadOneByte(EE_READ_ADDR, SYS_BD_TX_ADDR);
	*ptr += u8Temp;
}

void ReadBdSendSucceedNbr(UINT16 *ptr)
{
    UINT8 u8Temp;

	u8Temp = I2CMasterReadOneByte(EE_READ_ADDR, SYS_BD_TX_SUCC+1);
	*ptr = (UINT16)u8Temp << 8;

	u8Temp = I2CMasterReadOneByte(EE_READ_ADDR, SYS_BD_TX_SUCC);
	*ptr += u8Temp;
}

void SaveBdRxNbr(UINT16 *ptr)
{
  UINT8 *pBuf;

  pBuf = (UINT8 *)ptr;
  I2CMasterWrite(EE_WRITE_ADDR, SYS_BD_RX_ADDR, &pBuf[0], 2);
}
void SaveBdSendNbr(UINT16 *ptr)
{
  UINT8 *pBuf;

  pBuf = (UINT8 *)ptr;
  I2CMasterWrite(EE_WRITE_ADDR, SYS_BD_TX_ADDR, &pBuf[0], 2);
}

void SaveBdSendSucceedNbr(UINT16 *ptr)
{
  UINT8 *pBuf;

  pBuf = (UINT8 *)ptr;
  I2CMasterWrite(EE_WRITE_ADDR, SYS_BD_TX_SUCC, &pBuf[0], 2);
}
#endif

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
  if(0x55 != u8Temp)                                        /* 设备初次使用，存储器默认为0xff，则进入默认设置 */
  {
    //读取公司的编码
    ReadCompanyNbr(&sRecordCb.u16CmpNbr);
    //读取设备的编号
    ReadSystemNbr(&sRecordCb.u16SysNbr);  //注意大小端模式

    //将CDMA和北斗卡号清零
    memset(&sSendInfo.au8CdmaIndex[0], 0, 11);
    memset(&sSendInfo.au8BDIndex[0], 0, 11);

	sRecordCb.u8SysType  = SYS_TYPE_WYL;
    sRecordCb.u8GapTime  = '1';     //默认为5分钟

    memset(sRecordCb.au8BdRevNbr, 0, 3);
	memset(sRecordCb.au8BdBckNbr, 0, 3);

	memset(sRecordCb.au8CdmaRevNbr, 0, 11);
	memset(sRecordCb.au8CdmaBckNbr, 0, 11);

    sSendInfo.u8MsgSource = AUTO_COM_MODE;

    sRecordCb.u32SysRunTime = 0;

#ifdef BEIDOU_TEST
    u16BdRxNum   = 0;
    u16BdSendNum = 0;
    u16BdSendSucceed =0;
#endif

    I2CMasterWriteOneByte(EE_WRITE_ADDR, SYS_CFG_ADDR, 0x55);
  }
  else
  {
    //读取公司的编码
    ReadCompanyNbr(&sRecordCb.u16CmpNbr);
    //读取设备的编号
    ReadSystemNbr(&sRecordCb.u16SysNbr);  //注意大小端模式

    //通讯方式选择
    sSendInfo.u8MsgSource = ReadCommunicationMode();

    //读取设备的类型
    ReadSystemType(&sRecordCb.u8SysType);

    //时间间隔
    ReadSystemGapTime(&sRecordCb.u8GapTime);

	//读取北斗接收机号码
	ReadBdReceivingNbr(&sRecordCb.au8BdRevNbr[0]);
//    ReadSystemBdIndex(&sSendInfo.au8BDIndex[0], &sRecordCb.au8BdRevNbr[0]);
    //读取备用北斗接收机号码
	ReadBckBdReceivingNbr(&sRecordCb.au8BdBckNbr[0]);

	//读取CDMA接收机号码
	ReadCdmaReceivingNbr(&sRecordCb.au8CdmaRevNbr[0]);
    //获取CDMA本机号码
    ReadCdmaLocationNbr(&sSendInfo.au8CdmaIndex[0]);
    //读取CDMA接收机号码
	ReadBckCdmaReceivingNbr(&sRecordCb.au8CdmaBckNbr[0]);

//    ReadSystemRunTime(&sRecordCb.u32SysRunTime);
    sRecordCb.u32SysRunTime = 0;       //每次启动系统后，运行时间清零
    SaveSystemRunTime(&sRecordCb.u32SysRunTime);

#ifdef BEIDOU_TEST
    //读取北斗接收到报文的次数
	  ReadBdRxNbr(&u16BdRxNum);
    //读取北斗发送报文的次数
	  ReadBdSendNbr(&u16BdSendNum);
    //读取北斗发送成功次数
    ReadBdSendSucceedNbr(&u16BdSendSucceed);
#endif
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
      SaveCompanyNbr(&sRecordCb.u16CmpNbr);
      SaveSystemNbr(&sRecordCb.u16SysNbr);

      SaveCommunicationMode(sSendInfo.u8MsgSource);
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
    if(g_u8NeedSaved & SAVE_BD_NBR)
    {
       SaveBdReceivingNbr(&sRecordCb.au8BdRevNbr[0]);
       SaveBdBckReceivingNbr(&sRecordCb.au8BdBckNbr[0]);
    }

    //保存短信接收机号码
    if(g_u8NeedSaved & SAVE_CDMA_NBR)
    {
      SaveCdmaReceivingNbr(&sRecordCb.au8CdmaRevNbr[0]);
      SaveCdmaBckReceivingNbr(&sRecordCb.au8CdmaBckNbr[0]);
      //保存本机号码到存储器
      SaveSystemCdmaIndex(&sSendInfo.au8CdmaIndex[0]);
    }

    g_u8NeedSaved = SAVE_NONE;
  }
}




