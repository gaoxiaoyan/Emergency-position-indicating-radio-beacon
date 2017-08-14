

#ifndef __DTUMODULE_H__
#define __DTUMODULE_H__

#include "DataType.h"

//DTU���÷���״̬
#define DTU_CFG_MODE          0u
#define DTU_CFG_OK            1u
#define DTU_CFG_ERROR         2u
#define DTU_UART_SET_ERROR    3u
#define DTU_TCP_SET_ERROR     4u
#define DTU_AUTO_SET_ERROR    5u
#define DTU_ID_SET_ERROR      6u
#define DTU_AGAIN_SET_ERROR   7u
#define DTU_IDLE_SET_ERROR    8u
#define DTU_BEAT_SET_ERROR    9u
#define DTU_GAP_SET_ERROR     10u
#define DTU_MUL_SET_ERROR     11u
#define DTU_DCS_SET_ERROR     12u
#define DTU_MODE_SET_ERROR    13u
#define DTU_DEB_SET_ERROR     14u
#define DTU_IP_SUB1_ERROR     15u
#define DTU_IP_SUB2_ERROR     15u
#define DTU_IP_SUB3_ERROR     15u
#define DTU_IP_SUB4_ERROR     15u

/******************************************************************************
 * �������ƣ�GetDtuSvr
 * ��������ȡDTU������״̬
 *
 * ��������
 * ���أ�bFlag�� 1 -- ���ߡ�0 -- û������
 *****************************************************************************/
extern BOOL GetDtuSvr(void);

/******************************************************************************
 * �������ƣ�DtuInfoSend
 * ������������Ϣ�����ġ�
 *
 * ��������
 * ���أ���
 *****************************************************************************/
extern void DtuInfoSend(void);

/******************************************************************************
 * �������ƣ�DtuCmdInit
 * ������DTU���������ʼ��
 *
 * ��������
 * ���أ���
 *****************************************************************************/
extern void DtuCmdInit(void);

/******************************************************************************
 * �������ƣ�DtuCmdRun
 * ������DTU�������
 *
 * ��������
 * ���أ���
 *****************************************************************************/
extern void DtuCmdRun(void);

/*****************************************************************************
 �� �� ��  : ConfigDtuModule
 ��������  : Config dtu module, 

 �������  : UINT8 u8State
 �� �� ֵ  :
*****************************************************************************/
extern void ConfigDtuModule(UINT8 u8State);

/*****************************************************************************
 �� �� ��  : ConfigDtuModule
 ��������  : Config dtu module, 

 �������  : UINT8 u8State
 �� �� ֵ  :
*****************************************************************************/
extern UINT8 McuConfigDtuMainServer(void);

/*****************************************************************************
 �� �� ��  : McuConfigDtuSub1
 ��������  : Config dtu module, 

 �������  : UINT8 u8State
 �� �� ֵ  :
*****************************************************************************/
extern UINT8 McuConfigDtuSub1(UINT8 u8CfgEnable);

/*****************************************************************************
 �� �� ��  : McuConfigDtuSub2
 ��������  : Config dtu module, 

 �������  : UINT8 u8State
 �� �� ֵ  :
*****************************************************************************/
extern UINT8 McuConfigDtuSub2(UINT8 u8CfgEnable);

/*****************************************************************************
 �� �� ��  : McuConfigDtuSub3
 ��������  : Config dtu module, 

 �������  : UINT8 u8State
 �� �� ֵ  :
*****************************************************************************/
extern UINT8 McuConfigDtuSub3(UINT8 u8CfgEnable);

/*****************************************************************************
 �� �� ��  : McuConfigDtuSub4
 ��������  : Config dtu module, 

 �������  : UINT8 u8State
 �� �� ֵ  :
*****************************************************************************/
extern UINT8 McuConfigDtuSub4(UINT8 u8CfgEnable);

/*****************************************************************************
 �� �� ��  : GetDtuConfigInfo
 ��������  : Get DTU module config, Here mainly get module ID.

 �������  : none
 �� �� ֵ  : ����״̬
*****************************************************************************/
extern UINT8 GetDtuConfigInfo(UINT8 *pu8Buf);

#endif