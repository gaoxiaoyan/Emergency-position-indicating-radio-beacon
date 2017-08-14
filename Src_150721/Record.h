
#ifndef __RECORD_H__
#define __RECORD_H__

#include "Communication.h"

#ifdef  RECORD_GLOBAL
#define RECORD_EXT    
#else
#define RECORD_EXT extern
#endif

//�洢�豸������ַ
#define EE_WRITE_ADDR       0xa0
#define EE_READ_ADDR        0xa1

#define SYS_TYPE_WYL        BIT0
#define SYS_TYPE_SYJ        BIT1
    
//����ʹ�ܱ�־λ
#define SAVE_NONE           0x00
#define SAVE_SYS_NBR        BIT0
#define SAVE_SYS_TYPE       BIT1
#define SAVE_SYS_GAP        BIT2
#define SAVE_REV_BD_NBR     BIT3
#define SAVE_BCK_BD_NBR     BIT4
#define SAVE_REV_CDMA_NBR   BIT5
#define SAVE_BCK_CDMA_NBR   BIT6
#define SAVE_ALL_CFG        (BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0)

//������洢��ַ��Ԥ��0x00 ~ 0x0f
#define SYS_CFG_ADDR        0x00    //�����������ã�0xffΪ�豸û�б����ã�0x55Ϊ�Ѿ�������
#define SYS_NBR_ADDR        0x10    //�豸����
#define SYS_TYPE_ADDR       0x20    //�豸����
#define SYS_GAP_ADDR        0x21    // ����ʱ��������
#define SYS_REV_BD_ADDR     0x30    //�������ջ�����
#define SYS_BCK_BD_ADDR     0x30    //���ñ������ջ�����
#define SYS_REV_CDMA_ADDR   0x40    //cdma���ջ�����
#define SYS_BCK_CDMA_ADDR   0x50    //���ý��ջ�����
#define SYS_RUN_TIME_ADDR   0x60    //��������ʱ�䣬UINT32���ͣ���λΪ������
#define SYS_BD_TX_ADDR      0x70    //�������ͱ��Ĵ���
#define SYS_BD_RX_ADDR      0x72    //�������ձ��Ĵ���
#define SYS_BD_TX_SUCC      0x74    //�������ͳɹ�����

typedef struct
{
  UINT16 u16SysNbr;         //�豸���
  UINT8  u8SysType;         //�豸����
  UINT8  u8GapTime;         //��Ϣ����ʱ��������߼��ʱ��255/60= 4.25H
  
  UINT8  au8BdRevNbr[3];    //�������ջ�����ָ��
  UINT8  au8BdBckNbr[3];    //�������ý��ջ�����ָ��
  
  UINT8  au8CdmaRevNbr[11];       //CDMA���ջ�����ָ��
  UINT8  au8CdmaBckNbr[11];       //cdma���ý��ջ�����ָ��
  
  UINT32  u32SysRunTime;          //��������ʱ��
}_tsRecordCb;

RECORD_EXT _tsRecordCb sRecordCb;


/********************************************************************************
 * �������ƣ�RecordInit.c
 * �������ܣ���ʼ����¼��Ϣ�������豸��ţ��豸��Ϣ��
 * 
 * ���룺 ��
 * ���أ� ��
 *  
 *******************************************************************************/
extern void RecordInit(void);

/********************************************************************************
 * �������ƣ�RecordRun
 * �������ܣ����մ��ڷ��͵��������ݣ�����ʾ����ʾ����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
extern void RecordRun(void);

/********************************************************************************
 * �������ƣ�ReadSystemNbr
 * �������ܣ���ȡ�豸�ı���
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
extern void ReadSystemNbr(UINT16 *ptr);

/********************************************************************************
 * �������ƣ�ReadSystemType
 * �������ܣ���ȡ�豸������
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
extern void ReadSystemType(UINT8 *ptr);

/********************************************************************************
 * �������ƣ�ReadBdReceivingNbr
 * �������ܣ���ȡ�������ջ�����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
extern void ReadBdReceivingNbr(UINT8 *ptr);

/********************************************************************************
 * �������ƣ�ReadBckBdReceivingNbr
 * �������ܣ���ȡ���ñ������ջ�����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
extern void ReadBckBdReceivingNbr(UINT8 *ptr);

/********************************************************************************
 * �������ƣ�ReadCdmaReceivingNbr
 * �������ܣ���ȡCDMA���ջ�����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
extern void ReadCdmaReceivingNbr(UINT8 *prt);

/********************************************************************************
 * �������ƣ�ReadBckCdmaReceivingNbr
 * �������ܣ���ȡ����CDMA���ջ�����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
extern void ReadBckCdmaReceivingNbr(UINT8 *ptr);

/********************************************************************************
 * �������ƣ�SaveSystemNbr
 * �������ܣ�����ϵͳ���
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
 extern void SaveSystemNbr(UINT16 *ptr);
 
 /********************************************************************************
 * �������ƣ�SaveSystemNbr
 * �������ܣ������豸����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
 extern void SaveSystemType(UINT8 *ptr);
 
 /********************************************************************************
 * �������ƣ�SaveBdReceivingNbr
 * �������ܣ����汱�����ջ�����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
 extern void SaveBdReceivingNbr(UINT8 *ptr);
 
 /********************************************************************************
 * �������ƣ�SaveBckBdReceivingNbr
 * �������ܣ����汸�ñ������ջ�����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
 extern void SaveBckBdReceivingNbr(UINT8 *ptr);

/********************************************************************************
 * �������ƣ�SaveBdReceivingNbr
 * �������ܣ�����cdma���ջ�����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
 extern void SaveCdmaReceivingNbr(UINT8 *ptr);
 
 /********************************************************************************
 * �������ƣ�SaveBckCdmaReceivingNbr
 * �������ܣ�����cdma���ջ�����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/ 
 extern void SaveBckCdmaReceivingNbr(UINT8 *ptr);
 
/********************************************************************************
 * �������ƣ�SaveSystemRunTime
 * �������ܣ������������ʱ��
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
extern void SaveSystemRunTime(UINT32 *ptr);

/********************************************************************************
 * �������ƣ�ReadSystemRunTime
 * �������ܣ���ȡ�����ϴ�����ʱ��
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
extern void ReadSystemRunTime(UINT32 *ptr);

#ifdef BEIDOU_TEST
extern void ReadBdRxNbr(UINT16 *ptr);
extern void ReadBdSendNbr(UINT16 *ptr);
extern void ReadBdSendSucceedNbr(UINT16 *ptr);

extern void SaveBdRxNbr(UINT16 *ptr);
extern void SaveBdSendNbr(UINT16 *ptr);
extern void SaveBdSendSucceedNbr(UINT16 *ptr);
#endif

#endif