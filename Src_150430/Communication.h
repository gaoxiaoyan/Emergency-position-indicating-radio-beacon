
#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include "DataType.h"

typedef struct
{
  UINT8 u8Index;  //�������
  UINT8 *pGPS_Y;   //gpsγ��
  UINT8 *pGPS_X;   // gps����
  UINT8 u8Battery;  //��ص���
}_tsSendInfo;

extern _tsSendInfo sSendInfo;

/*****************************************************************************
 �� �� ��  : CommunicationInit
 ��������  : �������ݽṹ��ʼ��

 �������  : none
 �� �� ֵ  : none,
*****************************************************************************/
extern void CommunicationInit(void);

/*****************************************************************************
 �� �� ��  : SendInfoToCenter
 ��������  : �������ݵ����ģ�

 �������  : none
 �� �� ֵ  : none,
*****************************************************************************/
extern void SendInfoToCenter(void);

/****************************************************************
 * �������ƣ�ComModuleSwitch
 * ��    �ܣ��ж�CDMAģ���ź�ǿ�ȣ������㷢��Ҫ����ʹ��CDMA���ͣ�
             ����򿪱���ģ��
 *
 * ��    ������
 * ����ֵ  ����
****************************************************************/
extern void ComModuleSwitch(void);

#endif