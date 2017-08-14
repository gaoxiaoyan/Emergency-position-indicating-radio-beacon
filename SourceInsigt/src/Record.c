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
#include "msp430x54x.h"
#include "DataType.h"
#include "global.h"
#include "string.h"
#include "ctype.h"
#include "I2C_Driver.h"
#include "Record.h"
#include "SystemCtl.h"


/********************************************************************************
 * �������ƣ�ReadDebugSwtich
 * �������ܣ������Կ���
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void ReadDebugSwtich(void)
{
   g_bDebug = I2CMasterReadOneByte(EE_WRITE_ADDR, DEBUG_EN_ADDR);
}

/********************************************************************************
 * �������ƣ�WriteDebugSwtich
 * �������ܣ�д������Ϣ�ӿ�
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
 void SaveSystemNbr(void)
 {
   I2CMasterWriteOneByte(EE_WRITE_ADDR, );
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
    sRecordCb.
  }
  //��ȡ�������ջ�����
  ReadBdReceivingNbr();
  
  //��ȡCDMA���ջ�����
  ReadCdmaReceivingNbr();
 
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
      SaveSystemNbr();
    }
    
    //�����豸���͡�1 -- Χ������2 -- ���ͻ�
    if(g_u8NeedSaved & SAVE_SYS_TYPE)
    {
      SaveSystemType();
    }
  
    //���汱�����ջ�����
    if(g_u8NeedSaved & SAVE_REV_BD_NBR)
    {
       SaveBdReceivingNbr();
    }
    
    //������Ž��ջ�����
    if(g_u8NeedSaved & SAVE_REV_CDMA_NBR)
    { 
     SaveCdmaReceivingNbr();
    }
   
    g_u8NeedSaved = SAVE_NONE;
  }
}