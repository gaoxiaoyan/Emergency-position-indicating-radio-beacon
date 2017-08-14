
#ifndef __BDMODULE_H__
#define __BDMODULE_H__

#include "DataType.h"
#include "Communication.h"

#ifdef  BD_GLOBAL
#define BD_EXT
#else
#define BD_EXT extern
#endif

////////////////////////////////////////////////////////////////////////////
//                     ��������ṹ
//$DWSQͨѶ����ṹ
typedef struct
{
  UINT8 au8BdCmd[5];          //����ģ��ͨѶ����
  UINT8 au8DataLen[2];        //���ݰ�����     
  UINT8 au8BdLocaAddr[3];     //�������û�����ַ����������
  
  UINT8 u8InfoType;           //��Ϣ���
  UINT8 au8HighData[4];       //�߳����ݺ����߸�
  UINT8 au8Pressure[4];       //��ѹ����
  UINT8 au8u8Freq[2];         //��վƵ��
  UINT8 u8Check;              //У���
}_tsCmdDwsqCb;
BD_EXT _tsCmdDwsqCb sCmdDwsqCb;

//$TXSQͨѶ��������ṹ
typedef struct
{
  UINT8 au8BdCmd[5];          //����ģ��ͨѶ����
  UINT8 au8DataLen[2];        //���ݰ�����     
  UINT8 au8BdLocaAddr[3];     //�������û�����ַ����������
  
  UINT8 u8InfoType;           //��Ϣ���
  UINT8 au8BdSendAddr[3];      //���������ջ���ַ��
  UINT8 au8MsgLen[2];         //���ĳ���
  UINT8 u8Ack;                //�Ƿ�Ӧ��
  UINT8 *pMsg;                //������Ϣָ��
  UINT8 u8Check;              //У���
}_tsCmdTxsqCb;
BD_EXT _tsCmdTxsqCb sCmdTxsqCb;

//$SJSCʱ���������ṹ
typedef struct
{
  UINT8 au8BdCmd[5];          //����ģ��ͨѶ����
  UINT8 au8DataLen[2];        //���ݰ�����     
  UINT8 au8BdLocaAddr[3];     //�������û�����ַ����������
  
  UINT8 au8Freq[2];           //���Ƶ��
  UINT8 u8Check;              //У���
}_tsCmdSjscCb;
BD_EXT _tsCmdSjscCb sCmdSjscCb;

////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//                    ��������ṹ����
//$GLJC���ʼ������ṹ
typedef struct
{
  UINT8 au8BdCmd[5];          //����ģ��ͨѶ����
  UINT8 au8DataLen[2];        //���ݰ�����     
  UINT8 au8BdLocaAddr[3];     //�������û�����ַ����������
  
  UINT8 u8Freq;               //���Ƶ��
}_tsCmdGljcCb;
BD_EXT _tsCmdGljcCb sCmdGljcCb;

typedef struct
{
  UINT8 au8BD_X[10];
  UINT8 au8BD_Y[9];
}_tsBdLocation;

//$DWXX��λ��Ϣ����ṹ
typedef struct
{
  UINT8 au8BdCmd[5];          //����ģ��ͨѶ����
  UINT8 au8DataLen[2];        //���ݰ�����     
  UINT8 au8BdLocaAddr[3];     //�������û�����ַ����������
  
  UINT8 u8InfoType;           //��Ϣ���
  UINT8 au8BdFindAddr[3];     //���������ջ���ַ��
  
  //λ������
  UINT8 au8TimeData[4];
  UINT8 au8BD_X[4];        //������Ϣ      
  UINT8 au8BD_Y[4];        //γ����Ϣ
  UINT8 au8HData[2];
  UINT8 au8uHData[2];
  
  UINT8 u8Check;              //У���
}_tsCmdDwxxCb;
BD_EXT _tsCmdDwxxCb sCmdDwxxCb;

//$TXXXͨ����Ϣ����ṹ
typedef struct
{
  UINT8 au8BdCmd[5];          //����ģ��ͨѶ����
  UINT8 au8DataLen[2];        //���ݰ�����     
  UINT8 au8BdLocaAddr[3];     //�������û�����ַ����������
  
  UINT8 u8InfoType;           //��Ϣ���
  UINT8 au8SendAddr[3];       //���ͷ���ַ
  UINT8 u8SendHour;           //����ʱ�䣬Сʱ
  UINT8 u8SendMinute;         //����ʱ�䣬���ӡ�
  
  UINT8 au8MsgLen[2];         //���ĳ���
  UINT8 *pMsg;                //�������ݵ�ַ
  UINT8 u8Crc;                //CRC��־
  
  UINT8 u8Check;              //У���
}_tsCmdTxxxCb;
BD_EXT _tsCmdTxxxCb sCmdTxxxCb;

//$SJXXʱ����Ϣ����ṹ
typedef struct
{
  UINT8 au8BdCmd[5];          //����ģ��ͨѶ����
  UINT8 au8DataLen[2];        //���ݰ�����     
  UINT8 au8BdLocaAddr[3];     //�������û�����ַ����������
  
  UINT8 au8Year[2];           //��
  UINT8 u8Month;              //��
  UINT8 u8Date;               //��
  UINT8 u8Hour;               //ʱ
  UINT8 u8Minuth;             //��
  UINT8 u8Second;             //��
  
  UINT8 u8Check;              //У���
}_tsCmdSjxxCb;
BD_EXT _tsCmdSjxxCb sCmdSjxxCb;

//$FKXX������Ϣ����ṹ
typedef struct
{
  UINT8 au8BdCmd[5];          //����ģ��ͨѶ����
  UINT8 au8DataLen[2];        //���ݰ�����     
  UINT8 au8BdLocaAddr[3];     //�������û�����ַ����������
  
  UINT8 u8BackInfo;           //������־
  UINT8 u8OtherInfo[4];       //������Ϣ, ���͵������
  
  UINT8 u8Check;              //У���
}_tsCmdFkxxCb;
BD_EXT _tsCmdFkxxCb sCmdFkxxCb;


/****************************************************************************
�������ƣ�BdModuleInit()
��    �ܣ�����ģ���ʼ��

��    ������
����ֵ  ����
*****************************************************************************/
extern void BdModuleInit(void);

/***************************************************************************
//�������ƣ�BdCmdRun
//�������������ڽ����������
//
//��������
//���أ���
***************************************************************************/
extern void BdCmdRun(void);

/****************************************************************************
�������ƣ�BdSendCmdTXSQ()
��    �ܣ�����ģ��ͨѶ����

��    ������
����ֵ  ����
*****************************************************************************/
extern void BdSendCmdTXSQ(UINT8 *pSendAddr, UINT8 *pMsg, UINT8 u8MsgLen);

/***********************************************************************************************
  �������ƣ�GetBeiDouIcNum()
  �������ܣ���ȡ�����ı�������

  ���룺��
  �������
***********************************************************************************************/
extern void GetBeiDouIcNum(UINT8 *ptr);

/***********************************************************************************************
  �������ƣ�GetBeiDouSendNum()
  �������ܣ���ȡ���ջ��ı�������

  ���룺��
  �������
***********************************************************************************************/
extern void GetBeiDouSendNum(UINT8 *ptr);

#ifdef BEIDOU_TEST
extern void GetBufBdInfo(void);
#endif
#endif