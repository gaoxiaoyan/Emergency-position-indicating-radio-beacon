

/*
  GPS ָ�����ͷ�ļ�
*/

#ifndef __GPS_PARSE_H__
#define __GPS_PARSE_H__

#include "msp430x54x.h"

#include "global.h"
#include "Uart2.h"
#include "string.h"

#ifdef    GPS_GLOBALS
#define   GPS_EXT
#else
#define   GPS_EXT   extern
#endif

GPS_EXT BOOL g_bGpsCompleted;
//��һ���ϵ�ʹ�ý��ձ�־
GPS_EXT BOOL  g_bGpsFirstRx;
GPS_EXT SINT8 g_s8GpsFirstDog;

/***********************************************************************************
�������ƣ�Uart0CmdInit()
�������ܣ�GPS��������ṹ��ʼ��

������    ��
���أ�    ��
***********************************************************************************/
extern void GpsCmdInit(void);

/**********************************************************************************************
 �������ƣ�GpsCmdRun
 �������ܣ�GPS ��������������ɼ�8����Ч���ݣ��ҳ����������õ�һ������

 ������ ��
 ���أ� ��
**********************************************************************************************/
extern void GpsCmdRun(void);


#endif



