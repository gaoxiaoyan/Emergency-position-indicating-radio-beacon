

#ifndef __DTUMODULE_H__
#define __DTUMODULE_H__

#include "global.h"

//CDMAģ�������ն�����
#define  MSG_MAX_SIZE       30

//CDMA_RI�жϽӿ�
#define CdmaRi                BIT7
#define CdmaRiDir             P1DIR
#define CdmaRiOut             P1OUT
#define CdmaRiIn              P1IN
#define CdmaRiIntFlag         P1IFG
#define CdmaRiIES             P1IES
#define CdmaRiIE              P1IE
#define CdmaRiIntCheck()      ((CdmaRiIntFlag & CdmaRi) == CdmaRi)
#define CdmaRiIntFlagClr()    (CdmaRiIntFlag &= ~CdmaRi)
#define CdmaRiIntEnable()     (CdmaRiIE  |=  CdmaRi)
#define CdmaRiIntDisable()    (CdmaRiIE  &= ~CdmaRi)
#define CdmaRiIesEdge()       (CdmaRiIES |=  CdmaRi)

#define CdmaDtr               BIT4
#define CdmaDtrDir            P4DIR
#define CdmaDtrOut            P4OUT
#define CdmaDtrSetOut()       (CdmaDtrDir |=  CdmaDtr)
#define CdmaDtrSetHigh()      (CdmaDtrOut |=  CdmaDtr)
#define CdmaDtrSetLow()       (CdmaDtrOut &= ~CdmaDtr)

//CDMA��Դ����
#define CdmaPower              BIT4
#define CdmaPowerOut           P1OUT
#define CdmaPowerDir           P1DIR
#define CdmaPowerOn()          CdmaPowerOut |=  CdmaPower
#define CdmaPowerOff()         CdmaPowerOut &= ~CdmaPower

#if DTIC_DET_EN > 0
#define CdmaIcCheck            BIT7
#define CdmaIcCheckDir         P2DIR
#define CdmaIcCheckOut         P2OUT
#define CdmaIcCheckIn          P2IN
#define CdmaIcCheckSetIn()     (CdmaIcCheckDir &= ~CdmaIcCheck)
#define CdmaIcDet()            ((CdmaIcCheckIn & CdmaIcCheck) == CdmaIcCheck)
#endif

//CDMA ON/OF
#define CdmaOnOff              BIT0
#define CdmaOnOffOut           P4OUT
#define CdmaOnOffDir           P4DIR
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

extern void CdmaToBeiDou(void);
extern void BeiDouToCdma(void);

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
 �� �� ��  : DeletCdmaMsg()
 ��������  : ����ɾ������

 �������  : s8Index -- ��Ҫɾ���Ķ��ű��, ���洢31������
                        31 -- ɾ��ȫ���Ķ���Ϣ

 �� �� ֵ  : TRUE, FALSE
*****************************************************************************/
extern BOOL DeletCdmaMsg(SINT8 s8Index);

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

/*****************************************************************************
 �� �� ��  : CdmaLowPowerEnter
 ��������  : CDMAģ�����͹���ģ��

 �������  : PTR
 �� �� ֵ  : TRUE , FALSE
*****************************************************************************/
extern void CdmaLowPowerEnter(void);

/*****************************************************************************
 �� �� ��  : CdmaLowPowerExit
 ��������  : CDMAģ���˳��͹���ģ��

 �������  : PTR
 �� �� ֵ  : TRUE , FALSE
*****************************************************************************/
extern void CdmaLowPowerExit(void);

/*****************************************************************************
 �� �� ��  : CdmaWakeUpCfg
 ��������  : CDMAģ���˳�˯��ģʽ������ȡCDMA�źţ��ź�ǿ�ȵ���16ʱ�Զ��л���
             ����ͨѶģ����

 �������  : PTR
 �� �� ֵ  : TRUE , FALSE
*****************************************************************************/
extern void CdmaWakeUpCfg(void);

/*****************************************************************************
 �� �� ��  : CdmaCmdRun
 ��������  : CDMA���ݽ���

 �������  : ��
 �� �� ֵ  : �� , ��
*****************************************************************************/
extern void CdmaCmdRun(void);

#endif
