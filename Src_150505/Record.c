/******************************************************************************
*											
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved										
*
* �� �� ���� Record.C
* ��    ���� ��¼

* �������ڣ� 2014��9��5��11:31:59
* ��    �ߣ� Bob
* ��ǰ�汾�� V1.00

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
 * �������ƣ�ReadSystemNbr
 * �������ܣ���ȡ�豸�ı���
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void ReadSystemNbr(UINT16 *ptr)
{
	UINT8 u8Temp;

	u8Temp = I2CMasterReadOneByte(EE_READ_ADDR, SYS_NBR_ADDR);
	*ptr = (UINT16)(u8Temp << 8);

	u8Temp = I2CMasterReadOneByte(EE_READ_ADDR, SYS_NBR_ADDR+1);
	*ptr += u8Temp;
}

/********************************************************************************
 * �������ƣ�ReadSystemType
 * �������ܣ���ȡ�豸������
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void ReadSystemType(UINT8 *ptr)
{
	*ptr = I2CMasterReadOneByte(EE_READ_ADDR, SYS_TYPE_ADDR);
}

/********************************************************************************
 * �������ƣ�ReadBdReceivingNbr
 * �������ܣ���ȡ�������ջ�����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void ReadBdReceivingNbr(UINT8 *ptr)
{
	ptr[0] = I2CMasterReadOneByte(EE_READ_ADDR, SYS_REV_BD_ADDR  );
	ptr[1] = I2CMasterReadOneByte(EE_READ_ADDR, SYS_REV_BD_ADDR+1);
	ptr[2] = I2CMasterReadOneByte(EE_READ_ADDR, SYS_REV_BD_ADDR+2);
}

/********************************************************************************
 * �������ƣ�ReadBckBdReceivingNbr
 * �������ܣ���ȡ���ñ������ջ�����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void ReadBckBdReceivingNbr(UINT8 *ptr)
{
	ptr[0] = I2CMasterReadOneByte(EE_READ_ADDR, SYS_BCK_BD_ADDR  );
	ptr[1] = I2CMasterReadOneByte(EE_READ_ADDR, SYS_BCK_BD_ADDR+1);
	ptr[2] = I2CMasterReadOneByte(EE_READ_ADDR, SYS_BCK_BD_ADDR+2);
}

/********************************************************************************
 * �������ƣ�ReadCdmaReceivingNbr
 * �������ܣ���ȡCDMA���ջ�����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void ReadCdmaReceivingNbr(UINT8 *ptr)
{
	I2CMasterRead(EE_READ_ADDR, SYS_REV_CDMA_ADDR, ptr, 11);
}

/********************************************************************************
 * �������ƣ�ReadBckCdmaReceivingNbr
 * �������ܣ���ȡ����CDMA���ջ�����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void ReadBckCdmaReceivingNbr(UINT8 *ptr)
{
  I2CMasterRead(EE_READ_ADDR, SYS_BCK_CDMA_ADDR, ptr, 11);
}

/********************************************************************************
 * �������ƣ�SaveSystemNbr
 * �������ܣ�����ϵͳ���
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
 void SaveSystemNbr(UINT16 *ptr)
 {
 	UINT8 *pBuf;

	pBuf = (UINT8 *)ptr;
  I2CMasterWrite(EE_WRITE_ADDR, SYS_NBR_ADDR, pBuf, 2);
 }

/********************************************************************************
 * �������ƣ�SaveSystemNbr
 * �������ܣ������豸����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
 void SaveSystemType(UINT8 *ptr)
 {
 	UINT8 *pBuf;

	pBuf = ptr;
  I2CMasterWrite(EE_WRITE_ADDR,SYS_TYPE_ADDR, pBuf, 1);
 }

/********************************************************************************
 * �������ƣ�SaveBdReceivingNbr
 * �������ܣ����汱�����ջ�����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void SaveBdReceivingNbr(UINT8 *ptr)
{
	I2CMasterWrite(EE_WRITE_ADDR,SYS_REV_BD_ADDR, ptr, 3);
}

 /********************************************************************************
 * �������ƣ�SaveBckBdReceivingNbr
 * �������ܣ����汸�ñ������ջ�����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void SaveBckBdReceivingNbr(UINT8 *ptr)
{
	I2CMasterWrite(EE_WRITE_ADDR, SYS_BCK_BD_ADDR, ptr, 3);
}

/********************************************************************************
 * �������ƣ�SaveBdReceivingNbr
 * �������ܣ�����cdma���ջ�����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void SaveCdmaReceivingNbr(UINT8 *ptr)
{
	I2CMasterWrite(EE_WRITE_ADDR,SYS_REV_CDMA_ADDR, ptr, 11);
}

/********************************************************************************
 * �������ƣ�SaveBckCdmaReceivingNbr
 * �������ܣ�����cdma���ջ�����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void SaveBckCdmaReceivingNbr(UINT8 *ptr)
{
	I2CMasterWrite(EE_WRITE_ADDR,SYS_BCK_CDMA_ADDR, ptr, 11);
}


/********************************************************************************
 * �������ƣ�RecordInit
 * �������ܣ���Ϣ��ʼ��
 *
 * ���룺 ��
 * ���أ� ��
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

		memset(sRecordCb.au8BdRevNbr, 0, 3);
		memset(sRecordCb.au8BdBckNbr, 0, 3);

		memset(sRecordCb.au8CdmaRevNbr, 0, 11);
		memset(sRecordCb.au8CdmaBckNbr, 0, 11);
  }
	else
	{ 
    //��ȡ�豸�ı��
    ReadSystemNbr(&sRecordCb.u16SysNbr);
    
    //��ȡ�豸������
    ReadSystemType(&sRecordCb.u8SysType);
    
	  //��ȡ�������ջ�����
	  ReadBdReceivingNbr(&sRecordCb.au8BdRevNbr[0]);
	  
	  //��ȡCDMA���ջ�����
	  ReadCdmaReceivingNbr(&sRecordCb.au8CdmaRevNbr[0]);
	}
 
}

/********************************************************************************
 * �������ƣ�RecordRun
 * �������ܣ����մ��ڷ��͵��������ݣ�����ʾ����ʾ����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void RecordRun(void)
{
  if((g_u8NeedSaved & SAVE_ALL_CFG) != SAVE_NONE)
  {
    //����ϵͳ���
    if(g_u8NeedSaved & SAVE_SYS_NBR)
    {
      SaveSystemNbr(&sRecordCb.u16SysNbr);
    }
    
    //�����豸���͡�1 -- Χ������2 -- ���ͻ�
    if(g_u8NeedSaved & SAVE_SYS_TYPE)
    {
      SaveSystemType(&sRecordCb.u8SysType);
    }
  
    //���汱�����ջ�����
    if(g_u8NeedSaved & SAVE_REV_BD_NBR)
    {
       SaveBdReceivingNbr(&sRecordCb.au8BdRevNbr[0]);
    }
    
    //������Ž��ջ�����
    if(g_u8NeedSaved & SAVE_REV_CDMA_NBR)
    { 
      SaveCdmaReceivingNbr(&sRecordCb.au8CdmaRevNbr[0]);
    }
   
    g_u8NeedSaved = SAVE_NONE;
  }
}




