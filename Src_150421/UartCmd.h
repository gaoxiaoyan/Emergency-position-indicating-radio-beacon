
/******************************************************************************
*											
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved										
*
* �� �� ���� UartCmd.h
* ��       ����

* �������ڣ� 2014��12��5��11:31:59
* ��    �ߣ� Bob
* ��ǰ�汾�� V1.00

------------------------------------------------------
*******************************************************************************/
#ifndef __UART_CMD_H__
#define __UART_CMD_H__

//����������ͣ�4·UART�Ľ�������
#define TYPE_ERROR              0u
#define GPS_TYPE                1u
#define DTU_TYPE                2u
#define USB_TYPE                3u
#define WIRELESS                4u

//��������У��
#define CMD_NONE               5u
#define CMD_XXPZ_OK            6u
#define CMD_XXPZ_ERROR         7u
#define CMD_DMPZ_OK            8u
#define CMD_DMPZ_ERROR         9u
#define CMD_S1PZ_OK            10u
#define CMD_S1PZ_ERROR         11u
#define CMD_S2PZ_OK            12u
#define CMD_S2PZ_ERROR         13u
#define CMD_S3PZ_OK            14u
#define CMD_S3PZ_ERROR         15u
#define CMD_S4PZ_OK            16u
#define CMD_S4PZ_ERROR         17u
#define CMD_ZDQD_OK            18u
#define CMD_ZDQD_ERROR         19u
#define CMD_ZDTZ_OK            20u
#define CMD_ZDTZ_ERROR         21u
#define CMD_ZDGJ_OK            22u
#define CMD_ZDGJ_ERROR         23u
#define CMD_TSKG_OK            24u
#define CMD_TSKG_ERROR         25u
#define CMD_BBHM_OK            26u
#define CMD_BBHM_ERROR         27u
#define CMD_DTHM_OK            28u
#define CMD_DTHM_ERROR         29u

//���ص�����״̬
#define DTU_CFG_OK                  1u
#define CFG_CHECK_ERROR             2u
#define CFG_ERROR                   3u

//
#define PZNON             0u    //������
#define XXPZ              1u    //��Ϣ����
#define DMPZ              2u    //������������
#define S1PZ              3u    //����1����
#define S2PZ              4u    //����2����
#define S3PZ              5u    //����3����
#define S4PZ              6u    //����3����
#define ZDQD              7u    //�豸����
#define ZDTZ              8u    //�豸ֹͣ
#define ZDGJ              9u    //�豸�ػ�
#define TSKG              10u   //���Կ���
#define BBHM              11u   //�汾����
#define DTHM              12u   //��ȡdtu������Ϣ
#define YJCX              13u


//����ָ֡����
#define XXPZ_DATA_LEN               1u
#define XXPZ_TML_INDEX              2u
//#define XXPZ_TML_TYPE               3u
#define XXPZ_TML_INFO               3u
#define XXPZ_SEND_TIME              4u
#define XXPZ_RESERVED               5u
#define XXPZ_CHECK_SUM              6u

//����ʹ��״̬
#define DTU_CFG_DISABLE             0u
#define DTU_CFG_ENABLE              1u
#define DTU_CFG_VIEW                2u

//����������������
#define DMPZ_CFG_LEN                1u
#define DMPZ_CFG_ID                 2u
#define DMPZ_CFG_EN                 3u
#define DMPZ_CFG_TU                 4u
#define DMPZ_CFG_APN                5u
#define DMPZ_CFG_NAME               6u
#define DMPZ_CFG_PWD                7u
#define DMPZ_CFG_IP                 8u
#define DMPZ_CFG_COM                9u
#define DMPZ_CFG_RESERVED           10u
#define DMPZ_CFG_CHK                11u

//������1����
#define S1PZ_CFG_LEN                1u
#define S1PZ_CFG_EN                 2u
#define S1PZ_CFG_IP                 3u
#define S1PZ_CFG_COM                4u
#define S1PZ_CFG_RESERVED           5u
#define S1PZ_CFG_CHK                6u

//������2����
#define S2PZ_CFG_LEN                1u
#define S2PZ_CFG_EN                 2u
#define S2PZ_CFG_IP                 3u
#define S2PZ_CFG_COM                4u
#define S2PZ_CFG_RESERVED           5u
#define S2PZ_CFG_CHK                6u

//������3����
#define S3PZ_CFG_LEN                1u
#define S3PZ_CFG_EN                 2u
#define S3PZ_CFG_IP                 3u
#define S3PZ_CFG_COM                4u
#define S3PZ_CFG_RESERVED           5u
#define S3PZ_CFG_CHK                6u

//������4����
#define S4PZ_CFG_LEN                1u
#define S4PZ_CFG_EN                 2u
#define S4PZ_CFG_IP                 3u
#define S4PZ_CFG_COM                4u
#define S4PZ_CFG_RESERVED           5u
#define S4PZ_CFG_CHK                6u

#define TSKG_CFG                    1u

#define IP_SUB_1            0
#define IP_SUB_2            1
#define IP_SUB_3            2
#define IP_SUB_4            3

#if 1
typedef struct
{
//  BOOL  bCfgEnable;               //����ʹ��
  BOOL  bCfgTcpUdp;               //TCP/UDPЭ���л���0--TCP/1--UDP
  BOOL  bCfgApn;                  //APN�룬0--�й��ƶ���1--�й���ͨ
  UINT8 u8CfgUserNameLen;        //�û�������
  UINT8 au8CfgUserName[16];      //�û�����
  UINT8 u8CfgPasswordLen;        //�û����볤��
  UINT8 au8CfgPassword[16];      //�û�����
  UINT8 u8CfgIpLen;              //ip��ַ����
  UINT8 au8CfgIpAddr[64];        //IP��ַ
  UINT8 au8CfgComIndex[2];       //�˿ں�
  UINT8 au8CfgId[11];            // -- ����λ��Ϊ105��byte
  
  BOOL  bBckTcpUdp;               //����TCP/UDPЭ���л���0--TCP/1--UDP
  BOOL  bBckApn;                  //����APN�룬0--�й��ƶ���1--�й���ͨ
  UINT8 u8BckUserNameLen;         //�����û�������
  UINT8 au8BckUserName[16];      //�����û�����
  UINT8 u8BckPasswordLen;        //�������볤��
  UINT8 au8BckPassword[16];      //�����û�����
  UINT8 u8BckIpLen;              //ip��ַ����
  UINT8 au8BckIpAddr[64];        //����IP��ַ
  UINT8 au8BckComIndex[2];        //���ݶ˿ں� 
  UINT8 au8BckId[11];             // -- �ӱ�������ʼ����λ��Ϊ105��byte
  
}_tsMainServer;

extern _tsMainServer sMainServer;
#endif

//DTU����
typedef struct
{
//  BOOL    bCfgEnable;          //IP����ʹ��
  
  UINT8   au8CfgIpAddr[60];    //IP��ַ
  UINT8   u8CfgIpLen;          //IP��ַ����
  BOOL    bCfgEnable;          //����ʹ��
  UINT8   au8CfgComIndex[2];   //�˿ں�
  
  UINT8   au8BckIpAddr[60];    //����IP��ַ
  UINT8   u8BckIpLen;          //����IP��ַ����
  BOOL    bBckEnable;          //����ʹ��
  UINT8   au8BckComIndex[2];   //���ݶ˿ں�
}_tsDtuCfg;

//����5��DTU����
extern _tsDtuCfg sDtuCfg[4];

//���ջ�������󳤶�
#define MAX_BODY_SIZE		100u

//����GPS���ջ�����
extern UINT8 au8GpsBodyBuf[MAX_BODY_SIZE];
//����DTU���ջ�����
extern UINT8 au8DtuBodyBuf[MAX_BODY_SIZE];
//����USB���ջ�����
extern UINT8 au8UsbBodyBuf[MAX_BODY_SIZE];

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
  UINT8 *pu8AssembleBuf;
		
  UINT8 u8BufDataLength;
	
  teUartCmdAssembleState eAssembleState;
	
}_tsUartCmdCb;

extern _tsUartCmdCb sUartCmdCb[4];

/***************************************************************************
//�������ƣ�UartCmdInit
//�������������ڽ����������
//
//��������
//���أ���
***************************************************************************/
extern void UartCmdInit(void);

/***************************************************************************
//�������ƣ�UartCmdRun
//�������������ڽ����������
//
//��������
//���أ���
***************************************************************************/
extern void UartCmdRun(void);

/***********************************************************************************
  �������ƣ�CreatCmdIndex
  �������ܣ�Ѱ�Ҷ���λ�ã�����������������ԭ�еĶ����滻Ϊ0x00

  ���룺pu8Buf -- ����GPS���ݵĻ�����
  �������
***********************************************************************************/
extern void CreatCmdIndex(uint8 * pu8Buf);

/***********************************************************************************
  �������ƣ�RealCmdProcess
  �������ܣ�����GPS���ݵ�N��������ƫ��

  ���룺����������Ľ���GPS���ݻ�����
  ��������ص�N��","֮�����Ϣ����Ҫ*buffer��Ч������������ſ���ִ��
***********************************************************************************/
extern uint8* RealCmdProcess( uint8* pu8Buf, uint8 num );

/**********************************************************************************************
 �������ƣ�ConfigInfoProcess
 �������ܣ����������������������Ϣcopy����Ϣ���ݽṹ���򿪱��档

 ������ u8Index   ��Ҫ������ͨ����USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 ���أ� У���״̬
**********************************************************************************************/
extern UINT8 ConfigInfoProcess(UINT8 u8Index, UINT8 *pu8RxBuffer);

/**********************************************************************************************
 �������ƣ�ConfigPackType
 �������ܣ������������

 ������ ��
 ���أ� ��������
**********************************************************************************************/
extern SINT8 ConfigPackType(const UINT8 *pu8Buf, UINT8 u8Size);

/**********************************************************************************************
 �������ƣ�MainServerConfigProcess
 �������ܣ�DTU���������������

 ������ u8Index   ��Ҫ������ͨ����USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 ���أ� У���״̬
**********************************************************************************************/
extern UINT8 MainServerConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer);

/**********************************************************************************************
 �������ƣ�Sub1ConfigProcess
 �������ܣ�DTU������1�������

 ������ u8Index   ��Ҫ������ͨ����USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 ���أ� У���״̬
**********************************************************************************************/
extern UINT8 Sub1ConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer);

/**********************************************************************************************
 �������ƣ�Sub2ConfigProcess
 �������ܣ�DTU������1�������

 ������ u8Index   ��Ҫ������ͨ����USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 ���أ� У���״̬
**********************************************************************************************/
extern UINT8 Sub2ConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer);

/**********************************************************************************************
 �������ƣ�Sub3ConfigProcess
 �������ܣ�DTU������1�������

 ������ u8Index   ��Ҫ������ͨ����USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 ���أ� У���״̬
**********************************************************************************************/
extern UINT8 Sub3ConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer);

/**********************************************************************************************
 �������ƣ�Sub4ConfigProcess
 �������ܣ�DTU������1�������

 ������ u8Index   ��Ҫ������ͨ����USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 ���أ� У���״̬
**********************************************************************************************/
extern UINT8 Sub4ConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer);

/**********************************************************************************************
 �������ƣ� DebugProcess
 �������ܣ�debug�ӿڿ����رմ���

 ������ u8Index   ��Ҫ������ͨ����USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 ���أ� У���״̬
**********************************************************************************************/
extern UINT8 DebugProcess(UINT8 u8Index, UINT8 *pu8RxBuffer);


/**********************************************************************************************
 �������ƣ� GetVersionProcess
 �������ܣ���ȡ�汾��Ϣ

 ������ u8Index   ��Ҫ������ͨ����USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 ���أ� У���״̬
**********************************************************************************************/
extern UINT8 GetVersionProcess(UINT8 u8Index, UINT8 *pu8RxBuffer);


/**********************************************************************************************
 �������ƣ� PowerDownProcess
 �������ܣ��ն˹ػ�ָ��

 ������ u8Index   ��Ҫ������ͨ����USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 ���أ� У���״̬
**********************************************************************************************/
extern UINT8 PowerDownProcess(UINT8 u8Index, UINT8 *pu8RxBuffer);

/**********************************************************************************************
 �������ƣ� ReadDtuConfigProcess
 �������ܣ���ȡDTU�ı�ʾ��

 ������ u8Index   ��Ҫ������ͨ����USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 ���أ� У���״̬
**********************************************************************************************/
extern UINT8 ReadDtuConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer);

#endif