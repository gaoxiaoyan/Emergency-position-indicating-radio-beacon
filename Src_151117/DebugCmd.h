
/******************************************************************************
*
* Copyright (C) 2015, Sunic-ocean
* All Rights Reserved
*
* �� �� ���� DebugCmd.h
* ��       ����

* �������ڣ� 2015��4��29��16:41:13
* ��    �ߣ� Bob
* ��ǰ�汾�� V1.00

------------------------------------------------------
*******************************************************************************/
#ifndef __DEBUG_CMD_H__
#define __DEBUG_CMD_H__

#include "DataType.h"

#ifdef    DEBUG_GLOBALS
#define   DEBUG_EXT
#else
#define   DEBUG_EXT   extern
#endif

//�������
#define CMD_HAND_SHAKE            0x00    /* �������������115200     */
#define CMD_GET_VERSION           0x01    /* ��ȡ�豸�汾��Ϣ           */
#define CMD_ALL_INFO_PRINT        0x02    /* ��ȡ�����б�               */
#define CMD_SET_RTC_TIME          0X03    /* ����RTCʱ�䣬��CDMA�϶�ȡ  */
#define CMD_GET_RTC_TIME          0x04    /* ��ȡRTCʱ�䡣              */
#define CMD_GET_SYS_BAT           0x05    /* ��ȡϵͳʣ�����           */
#define CMD_SET_GAP_TIME          0x06    /* ���÷�����Ϣ���           */
#define CMD_GET_GAP_TIME          0x07    /* ��ȡ������Ϣ���           */
#define CMD_SET_SYS_INDEX         0x08    /* �����豸����               */
#define CMD_GET_SYS_INDEX         0x09    /* �����豸����               */
#define CMD_CLR_RUN_TIME          0x0a    /* �����������ʱ��           */
#define CMD_GET_RUN_TIME          0x0b    /* ��ȡ��������ʱ��           */
#define CMD_USE_BD_MODE           0x0c    /* ʹ�ñ���ģʽͨѶ           */
#define CMD_USE_CDMA_MODE         0x0d    /* ʹ��CDMAģʽͨѶ           */

#define CMD_GPS_INFO_CLOSE        0x10    /* GPS������Ϣ��              */
#define CMD_GPS_INFO_OPEN         0x11    /* GPS������Ϣ��              */

#define CMD_BD_GET_LOCATION       0x20    /* ������λ��Ϣ��ȡ           */
#define CMD_BD_GET_LOC_NUM        0x21    /* ��������������Ϣ��ȡ       */
#define CMD_BD_SET_REC_NUM        0x22    /* �������ջ����Ż�ȡ         */
#define CMD_BD_GET_REC_NUM        0x23    /* �������ջ���������         */
#define CMD_BD_SET_BCK_NUM        0x24    /* �������ջ����ÿ��Ż�ȡ     */
#define CMD_BD_GET_BCK_NUM        0x25    /* �������ջ����ÿ�������     */
#define CMD_BD_INFO_TEST          0x26    /* �������ͱ��Ĳ���           */
#define CMD_BD_GET_TIME           0x27    /* ����ʱ���ȡ               */
#define CMD_BD_GET_SEND_SUC       0x28    /* ��������ɹ��ʻ�ȡ         */
#define CMD_BD_CLR_SEND_SUC       0x29    /* ��������ɹ�������         */

#define CMD_CDMA_INFO_CLOSE       0x30    /* CDMA������Ϣ��             */
#define CMD_CDMA_INFO_OPEN        0x31    /* CDMA������Ϣ��             */
#define CMD_CDMA_GET_TIME         0x32    /* ��ȡCDMAʱ��               */
#define CMD_CDMA_SET_LOC_NUM      0x33    /* ���ñ�������               */
#define CMD_CDMA_GET_LOC_NUM      0x34    /* ��ȡ��������               */
#define CMD_CDMA_SET_SEND_NUM     0x35    /* ���÷��ͺ���               */
#define CMD_CDMA_GET_SEND_NUM     0x36    /* ��ȡ���ͺ���               */
#define CMD_CDMA_SET_BCK_NUM      0x37    /* ���ñ��÷��ͺ���           */
#define CMD_CDMA_GET_BCK_NUM      0x38    /* ��ȡ���÷��ͺ���           */
#define CMD_CDMA_GET_CSQ          0x39    /* ��ȡ�ź�ǿ��               */
#define CMD_CDMA_INFO_TEST        0x3a    /* ���Ͷ��Ų���               */
#define CMD_CDMA_DEL_MSG          0x3b    /* ɾ��CDMAָ������           */

/**********************************************************************************************
 �������ƣ�DebugCmdInit
 �������ܣ�Debug cmd parse init

 ������ ��
 ���أ� ��
**********************************************************************************************/
extern void DebugCmdInit(void);

/***************************************************************************
//�������ƣ�UartCmdRun
//�������������ڽ����������
//
//��������
//���أ���
***************************************************************************/
extern void DebugCmdRun(void);


#endif