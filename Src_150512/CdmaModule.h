

#ifndef __DTUMODULE_H__
#define __DTUMODULE_H__

#include "DataType.h"

//CDMA_RI�жϽӿ�
#define CdmaRi                BIT7
#define CdmaRiDir             P1DIR
#define CdmaRiIn              P1IN
#define CdmaRiIntFlag         P1IFG 
#define CdmaRiIES             P1IES
#define CdmaRiIE              P1IE
#define CdmaRiIntCheck        (CdmaRiIntFlag & CdmaRi)
#define CdmaRiIntEnable()     (CdmaRiIE | CdmaRi)
#define CdmaRiIntDisable()    (CdmaRiIE & (~CdmaRi))
#define CdmaRiIesEdge()       (CdmaRiIES | CdmaRi)

//CDMA��Դ����
#define CdmaPower              BIT1
#define CdmaPowerOut           P10OUT
#define CdmaPowerDir           P10DIR
#define CdmaPowerOn()          CdmaPowerOut |=  CdmaPower
#define CdmaPowerOff()         CdmaPowerOut &= ~CdmaPower

//�ֻ������
#define CdmaIcCheck            BIT7
#define CdmaIcCheckDir         P2DIR
#define CdmaIcCheckIn          P2IN
#define CdmaIcCheckSetIn()     (CdmaIcCheckDir &= ~CdmaIcCheck)
#define CdmaIcDet()            ((CdmaIcCheckIn & CdmaIcCheck) == CdmaIcCheck)

//CDMA ON/OF
#define CdmaOnOff              BIT0
#define CdmaOnOffOut           P3OUT
#define CdmaOnOffDir           P3DIR
#define SetCdmaOnOff(x)        (x)? (CdmaOnOffOut |=  CdmaOnOff) : (CdmaOnOffOut &= ~CdmaOnOff)

//CDMA reset��λ
#define CdmaReset              BIT1
#define CdmaResetOut           P3OUT
#define CdmaResetDir           P3DIR
#define SetCdmaReset(x)        (x)? (CdmaResetOut |=  CdmaReset) : (CdmaResetOut &= ~CdmaReset) 


/*****************************************************************************
 �� �� ��  : CdmaInit
 ��������  : CDMAģ���ʼ��

 �������  : none
 �� �� ֵ  : none,
*****************************************************************************/
extern void CdmaInit(void);

/*****************************************************************************
 �� �� ��  : AssembleCdmaMsg
 ��������  : AssembleCdmaMsg

 �������  : *pNum -- ���ͺ����ָ��
             *pBuf -- ������Ϣָ��

 �� �� ֵ  : TRUE, FALSE
*****************************************************************************/
extern BOOL AssembleCdmaMsg(UINT8 *pNum, UINT8 *pBuf, UINT8 u8Len);

/*****************************************************************************
 �� �� ��  : GetCdmaCSQ
 ��������  : ��ȡCDMA�ź�ǿ��

 �������  : none
 �� �� ֵ  : �ź�ǿ��,0~31
*****************************************************************************/
extern UINT8 GetCdmaCSQ(void);

/*****************************************************************************
 �� �� ��  : GetCdmaTime
 ��������  : ��ȡCDMAʱ��, ���õ�RTC

 �������  : *pTime��*pu8Len
 �� �� ֵ  : none
*****************************************************************************/
extern void GetCdmaTime(UINT8 *pTime, UINT8 *pu8Len);

/*****************************************************************************
 �� �� ��  : GetCdmaTime
 ��������  : ��ȡCDMAʱ��

 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
extern BOOL SetCdmaTimeToRtc(void);

/*****************************************************************************
 �� �� ��  : SetLocationNbr
 ��������  : ���ñ�������

 �������  : none
 �� �� ֵ  : TRUE , FALSE
*****************************************************************************/
extern BOOL SetLocationNbr(UINT8 *prt);

/*****************************************************************************
 �� �� ��  : GetLocationNbr
 ��������  : ���ñ�������

 �������  : NONE
 �� �� ֵ  : TRUE , FALSE
*****************************************************************************/
extern BOOL GetLocationNbr(UINT8 *ptr);


#endif