

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


//GPS ������Ϣ�ṹ����
typedef struct
{
  //��λ��Ч��־
  BOOL bActive;

  // UTCʱ��
  UINT8 UTC_Time[6];
  UINT8 UTC_Date[6];

  // γ��
  UINT8 GPS_X[10];
  // γ�Ȱ���
  UINT8 NorthSouth;

  // ����
  UINT8 GPS_Y[11];
  //���Ȱ���
  UINT8 EastWest;

  //�������ʣ�000.0 ~999.9��   ******************************************/
  UINT8 Speed[8];

  //���溽��000.0~359.9�� ���汱����Ϊ��׼��
  UINT8 Course[8];

  //ʹ����������
  UINT8 UseEphSum;

}_tsGPSInfo;


#define GPS_MAX_SIZE      8
//�ⲿ�ṹ��������Ҫ�������ջ�����������
GPS_EXT _tsGPSInfo sGPSBuf[GPS_MAX_SIZE];


//���ݰ�����Э��
typedef enum
{
  //�ȴ����ݵ���ʼ��־
  E_UART_CMD_RECEIVING_HEADER,
  //��������
  E_UART_CMD_RECEIVING_BODY,
  //������Ч
  E_UART_CMD_RECEIVING_VALID
		
}teUartCmdAssembleState;

/* ��������ṹ���� */
typedef struct
{
  UINT8 *pu8AssembleBuf;  //���������ָ��
  
  UINT8 *pu8RxDataBuf;    //����������ָ��
  
  BOOL bGpsParse;         //���ݽ�����־��TRUE--���ڽ�����FALSE -- ������� 
		
  UINT16 u16BufDataLen ;  //��������ݳ���
  
  UINT16 u16RxDataLen;    //���ձ��������ݳ���
	
  teUartCmdAssembleState eAssembleState;  //���״̬
  
  SINT8 sGpsDog;
	
}_tsUartCmdCb;

GPS_EXT _tsUartCmdCb sGpsCmdCb;


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



