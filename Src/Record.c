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
#include "Communication.h"


/********************************************************************************
 * �������ƣ�ReadCompanyNbr
 * �������ܣ���ȡ�豸�Ĺ�˾����
 *
 * ���룺 ��
 * ���أ� ��
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
 * �������ƣ�ReadSystemNbr
 * �������ܣ���ȡ�豸�ı���
 *
 * ���룺 ��
 * ���أ� ��
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
    if((*ptr > 99) || (*ptr < 1))
    {
        *ptr = SYS_TYPE_WYL;
    }
}

/********************************************************************************
 * �������ƣ�ReadSystemGapTime
 * �������ܣ���ȡ�豸����Ϣ���ͼ��
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void ReadSystemGapTime(UINT8 *ptr)
{
	*ptr = I2CMasterReadOneByte(EE_READ_ADDR, SYS_GAP_ADDR);
    //�����ʱ�䳬��Χ��������ΪĬ��
    if((*ptr < '1') && (*ptr > '6'))
    {
        *ptr = '1';
    }
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
 * �������ƣ�ReadCommunicationMode
 * �������ܣ���ȡͨѶ��ʽ 1 -- CDMA; 2 -- BD; 3 -- CDMA + BD�Զ�
 *
 * ���룺 ��
 * ���أ� UINT8 -- ͨѶģʽ
 *
 *******************************************************************************/
UINT8 ReadCommunicationMode(void)
{
    return(I2CMasterReadOneByte(EE_READ_ADDR, SYS_COM_SEL_ADDR) & 0x03);
}

/********************************************************************************
 * �������ƣ�SaveCommunicationMode
 * �������ܣ�����ͨѶ��ʽ BIT0 -- CDMA; BIT1 -- BD; BIT0+BIT1 -- CDMA + BD�Զ�
 *
 * ���룺 UINT8 -- ͨѶ��ʽ
 * ���أ� ��
 *
 *******************************************************************************/
void SaveCommunicationMode(UINT8 u8Mode)
{
    I2CMasterWriteOneByte(EE_WRITE_ADDR, SYS_COM_SEL_ADDR, (u8Mode & 0x03));
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

void ReadSystemCdmaIndex(UINT8 *ptr, UINT8 *pSource)
{
    memcpy(ptr, pSource, 11);
}

void ReadCdmaLocationNbr(UINT8 *ptr)
{
    I2CMasterRead(EE_READ_ADDR, SYS_LOC_CDMA_ADDR, ptr, 11);
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
 * �������ƣ�ReadSystemRunTime
 * �������ܣ���ȡ�����ϴ�����ʱ��
 *
 * ���룺 ��
 * ���أ� ��
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
 * �������ƣ�SaveCompanyNbr
 * �������ܣ����泧�ұ��
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
 void SaveCompanyNbr(UINT16 *ptr)
 {
 	UINT8 *pBuf;

	pBuf = (UINT8 *)ptr;
    I2CMasterWrite(EE_WRITE_ADDR, SYS_CMP_ADDR, &pBuf[0], 2);
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
    I2CMasterWrite(EE_WRITE_ADDR, SYS_NBR_ADDR, &pBuf[0], 2);
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
 * �������ƣ�SaveSystemGapTime
 * �������ܣ����淢��ʱ����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
 void SaveSystemGapTime(UINT8 *ptr)
 {
 	UINT8 *pBuf;

	pBuf = ptr;
  I2CMasterWrite(EE_WRITE_ADDR,SYS_GAP_ADDR, pBuf, 1);
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
 * �������ƣ�SaveBdBckReceivingNbr
 * �������ܣ����汱�����ջ�����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void SaveBdBckReceivingNbr(UINT8 *ptr)
{
	I2CMasterWrite(EE_WRITE_ADDR,SYS_BCK_BD_ADDR, ptr, 3);
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

void SaveSystemCdmaIndex(UINT8 *ptr)
{
    I2CMasterWrite(EE_WRITE_ADDR,SYS_LOC_CDMA_ADDR, ptr, 11);
}

void SaveCdmaLocationNbr(UINT8 *ptr)
{
	I2CMasterWrite(EE_WRITE_ADDR,SYS_LOC_CDMA_ADDR, ptr, 11);
}

/********************************************************************************
 * �������ƣ�SaveCdmaBckReceivingNbr
 * �������ܣ�����cdma���ݽ��ջ�����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void SaveCdmaBckReceivingNbr(UINT8 *ptr)
{
	I2CMasterWrite(EE_WRITE_ADDR,SYS_BCK_CDMA_ADDR, ptr, 11);
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
 * �������ƣ�SaveSystemRunTime
 * �������ܣ������������ʱ��
 *
 * ���룺 ��
 * ���أ� ��
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
  if(0x55 != u8Temp)                                        /* �豸����ʹ�ã��洢��Ĭ��Ϊ0xff�������Ĭ������ */
  {
    //��ȡ��˾�ı���
    ReadCompanyNbr(&sRecordCb.u16CmpNbr);
    //��ȡ�豸�ı��
    ReadSystemNbr(&sRecordCb.u16SysNbr);  //ע���С��ģʽ

    //��CDMA�ͱ�����������
    memset(&sSendInfo.au8CdmaIndex[0], 0, 11);
    memset(&sSendInfo.au8BDIndex[0], 0, 11);

	sRecordCb.u8SysType  = SYS_TYPE_WYL;
    sRecordCb.u8GapTime  = '1';     //Ĭ��Ϊ5����

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
    //��ȡ��˾�ı���
    ReadCompanyNbr(&sRecordCb.u16CmpNbr);
    //��ȡ�豸�ı��
    ReadSystemNbr(&sRecordCb.u16SysNbr);  //ע���С��ģʽ

    //ͨѶ��ʽѡ��
    sSendInfo.u8MsgSource = ReadCommunicationMode();

    //��ȡ�豸������
    ReadSystemType(&sRecordCb.u8SysType);

    //ʱ����
    ReadSystemGapTime(&sRecordCb.u8GapTime);

	//��ȡ�������ջ�����
	ReadBdReceivingNbr(&sRecordCb.au8BdRevNbr[0]);
//    ReadSystemBdIndex(&sSendInfo.au8BDIndex[0], &sRecordCb.au8BdRevNbr[0]);
    //��ȡ���ñ������ջ�����
	ReadBckBdReceivingNbr(&sRecordCb.au8BdBckNbr[0]);

	//��ȡCDMA���ջ�����
	ReadCdmaReceivingNbr(&sRecordCb.au8CdmaRevNbr[0]);
    //��ȡCDMA��������
    ReadCdmaLocationNbr(&sSendInfo.au8CdmaIndex[0]);
    //��ȡCDMA���ջ�����
	ReadBckCdmaReceivingNbr(&sRecordCb.au8CdmaBckNbr[0]);

//    ReadSystemRunTime(&sRecordCb.u32SysRunTime);
    sRecordCb.u32SysRunTime = 0;       //ÿ������ϵͳ������ʱ������
    SaveSystemRunTime(&sRecordCb.u32SysRunTime);

#ifdef BEIDOU_TEST
    //��ȡ�������յ����ĵĴ���
	  ReadBdRxNbr(&u16BdRxNum);
    //��ȡ�������ͱ��ĵĴ���
	  ReadBdSendNbr(&u16BdSendNum);
    //��ȡ�������ͳɹ�����
    ReadBdSendSucceedNbr(&u16BdSendSucceed);
#endif
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
      SaveCompanyNbr(&sRecordCb.u16CmpNbr);
      SaveSystemNbr(&sRecordCb.u16SysNbr);

      SaveCommunicationMode(sSendInfo.u8MsgSource);
    }

    //�����豸���͡�1 -- Χ������2 -- ���ͻ�
    if(g_u8NeedSaved & SAVE_SYS_TYPE)
    {
      SaveSystemType(&sRecordCb.u8SysType);
    }

    //���淢��ʱ����
    if(g_u8NeedSaved & SAVE_SYS_GAP)
    {
      SaveSystemGapTime(&sRecordCb.u8GapTime);
    }

    //���汱�����ջ�����
    if(g_u8NeedSaved & SAVE_BD_NBR)
    {
       SaveBdReceivingNbr(&sRecordCb.au8BdRevNbr[0]);
       SaveBdBckReceivingNbr(&sRecordCb.au8BdBckNbr[0]);
    }

    //������Ž��ջ�����
    if(g_u8NeedSaved & SAVE_CDMA_NBR)
    {
      SaveCdmaReceivingNbr(&sRecordCb.au8CdmaRevNbr[0]);
      SaveCdmaBckReceivingNbr(&sRecordCb.au8CdmaBckNbr[0]);
      //���汾�����뵽�洢��
      SaveSystemCdmaIndex(&sSendInfo.au8CdmaIndex[0]);
    }

    g_u8NeedSaved = SAVE_NONE;
  }
}




