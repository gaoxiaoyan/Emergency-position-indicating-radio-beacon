
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
  UINT8 u8Index;    //�������
  UINT8 *pGPS_Y;    //gpsγ��
  UINT8 *pGPS_X;    // gps����
  UINT8 u8Battery;  //��ص���
  UINT8 u8GapTime;  //���ͼ��ʱ��1~6����5~30���ӣ�����5����
}_tsSendInfo;

COM_EXT _tsSendInfo sSendInfo;

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