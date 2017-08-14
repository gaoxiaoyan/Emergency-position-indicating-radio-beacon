
/******************************************************************************
*
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved
*
* ÎÄ ¼þ Ãû£º CdmaModule.C
* Ãè    Êö£º CDMAÊý¾Ý½âÎöÄ£¿é

* ´´½¨ÈÕÆÚ£º 2015Äê4ÔÂ24ÈÕ10:15:40
* ×÷    Õß£º Bob
* µ±Ç°°æ±¾£º V1.00

------------------------------------------------------
*******************************************************************************/
#define  CDMA_EXT
#include "msp430x54x.h"
#include "global.h"
#include "CdmaModule.h"
#include "DataType.h"
#include "Uart1.h"
#include "Uart2.h"
#include "SystemCtl.h"
#include "string.h"
#include "Timer.h"
#include "SoftTimer.h"
#include "GpsParse.h"
#include "stdio.h"
#include "OS_MEM.h"
#include "Rtc.h"
#include "BeiDouModule.h"

//Ç°ÃæµÄÇ¿ÖÆÀàÐÍ×ª»¯È±Ê¡»áÔì³É¼ÆËã´íÎó
#define _GetWordFromAddr(pAddr)         (UINT16)(((UINT16)pAddr[0]) << 8) + pAddr[1]

#define _GetDoubleWordFromAddr(pAddr)   ((UINT32)(((UINT32)pAddr[0]) << 24) +    \
                                         (UINT32)(((UINT32)pAddr[1]) << 16) +    \
                                         (UINT32)(((UINT32)pAddr[2]) << 8) + pAddr[3])

//ÃüÁî×ÖÄ¸Ïà¼Ó£¬È¡µÍ8Î»×÷ÎªÖ¸Áî´úÂë
#define CMD_ZCANS         0x7F
#define CMD_ZCCNT         0x82
#define CMD_CMTI          0x2D
#define CMD_ZCEND         0x74
#define CMD_CSQ           0xE7
#define CMD_SMS           0x68
#define CMD_CMGR          0x29
#define CMD_CMGL		  0x23
#define CMD_NONE          0x00

#define CMD_ASSERT(a)    ((a == CMD_CMTI ) || \
                          (a == CMD_CMGR ) || \
                          (a == CMD_CSQ  ))


//CDMA½ÓÊÕ×´Ì¬
typedef enum
{
    //ÃüÁî½ÓÊÕÍ·
    E_CDMA_RECEIVING_HEADER,
    //½ÓÊÕµ½µÄÃüÁî
    E_CDMA_RECEIVING_CMD,
    //Êý¾ÝÖ÷Ìå
    E_CDMA_RECEIVING_BODY,
    //SMS READYÃüÁî
    E_CDMA_RECEIVING_SMS,
    //ÓÐÐ§±êÖ¾×´Ì¬
    E_CDMA_RECEIVING_VALID

}teCdmaAssembleState;

/* Êý¾Ý×é°ü½á¹¹ÉùÃ÷ */
typedef struct
{
  UINT8 *pu8AssembleBuf;  //×é°üÇøÊý¾ÝÖ¸Õë

  UINT8 u8Cmd;            //cdma½ÓÊÕÃüÁî

  UINT16 u16BufDataLen ;  //×é°üµÄÊý¾Ý³¤¶È

  UINT8 u8MsgEnvent;      //ÊÕµ½¶ÌÐÅºó£¬Ä£¿é»áÏòÖ÷»ú·¢ËÍÒ»ÏµÁÐÖ¸Áî£¬
                          //4ÌõÖ¸Áî·Ö±ð¶ÔÓ¦µÍ4Î»

  teCdmaAssembleState eAssembleState;  //×é°ü×´Ì¬

  SINT8 sCdmaDog;         //cdma½ÓÊÕÃüÁî³¬Ê±¶¨Ê±Æ÷

  SINT8 sCsqDog;          //CSQÐÅºÅÇ¿¶È»ñÈ¡Ê±¼ä¶¨Ê±Æ÷

}_tsCdmaCmdCb;

_tsCdmaCmdCb sCdmaCmdCb;

//µ±ÓÐÎ´¶Á¶ÌÐÅÊ±£¬ÔÚÏàÓ¦µÄÎ»ÖÃ±êÊ¶Îª1£¬·ñÔòÎª0
UINT8 au8MsgUnRead[MSG_MAX_SIZE];
BOOL  bMsgReady = FALSE;

//¶¨Òåµ±Ç°½ÓÊÕµ½µÄ¶ÌÐÅ±àºÅ
SINT8 s8CurrentIndex = -1;

//½øÈëDTUÅäÖÃÄ£Ê½
const UINT8 u8ConfigMode[]         = {"enter_config_mode\0"};
const UINT8 u8ConfigStart[]        = {0xff,0xff,0xff,0xff};
const UINT8 u8ConfigRxData[]       = {"aaaaaaaaaaaaaaaa\0"};

const UINT8 au8BackOk[3]           = {0x4f, 0x4b, 0x00};

//²éÑ¯Ä£¿éÐÍºÅ
const UINT8 au8GetModuleIndex[]    = {"AT+CGMM\r"};
//´®¿Ú²¨ÌØÂÊÉèÖÃ
const UINT8 au8UartSet[]           = {"AT+UART=9600,ON,8,N,1\r"};
//ÉèÖÃ32khzÉî¶ÈË¯ÃßÄ£Ê½
const UINT8 au8EnableZDSLEEP[]     = {"AT+ZDSLEEP=1\r"};
const UINT8 au8DisableZDSLEEP[]    = {"AT+ZDSLEEP=0\r"};
//DTRÒý½ÅÐÝÃß¿ØÖÆ
const UINT8 au8EnableZDSLEEPDTR[]  = {"AT+ZDSLEEPDTR=1\r"};
const UINT8 au8DisableZDSLEEPDTR[] = {"AT+ZDSLEEPDTR=0\r"};
//»ñÈ¡CDMAÊ±¼ä
const UINT8 au8GetCdmaTime[]       = {"AT+ZDSLEEPDTR=2\r"};
//¸´Î»Ä£¿é
const UINT8 au8DisableZRST[]       = {"AT+ZRST=0\r"};
//ÔÊÐí¸´Î»,1·ÖÖÓºó¸´Î»
//const UINT8 au8EnableZRST[] = {"AT+ZRST=1,"00:01"\r"};

//ÐÅºÅÇ¿¶È²éÑ¯£¬
//·µ»Ø£º+CSQ: 28,99 OK£¬16ÒÔÏÂ¿ÉÄÜºô½ÐÊ§°Ü¡£
const UINT8 au8CSQ[]               = {"AT+CSQ\r"};

//Ê±ÖÓ²éÑ¯
//·µ»Ø£º+CCLK:"2004/02/09£¬17£º34£º23.694"µ±Ç°ÍøÂçÊ±¼ä¡£
const UINT8 au8CCLK[]              = {"AT+CCLK?\r"};

//ÉèÖÃ¶ÌÏûÏ¢Ä£Ê½
const UINT8 au8CMGF[]              = {"AT+CMGF=1\r"};
//¶ÁÈ¡¶ÌÏûÏ¢
const UINT8 au8CMGR[]              = {"AT+CMGR="};
//¶Á¶ÌÐÅÁÐ±í
const UINT8 au8CMGL[]              = {"AT+CMGL="};

//ÉèÖÃ±¾»úºÅÂë
//const UINT8 au8SetCNUM[] = {"AT+CNUM="};
//const UINT8 au8GetCNUM[] = {"AT+CNUM?"};

//Ä£¿éÖ¸Ê¾
const UINT8 au8ZIND[] = {"AT+ZIND="};

/////////////////////////////////////////////////////////////
// ¶ÌÐÅ·þÎñ
//Ñ¡Ôñ¶ÌÐÅ·þÎñ
const UINT8 au8MsgType[]            = {"AT+CSMS?\r"};
//ÉèÖÃ¶ÌÐÅÎªÓ¢ÎÄ£¬ ascii ¸ñÊ½
const UINT8 au8ZMSGL_ASCII[]        = {"AT+ZMSGL=1,2\r"};
//ÉèÖÃ¶ÌÐÅÎªººÓï £¬ unicode¸ñÊ½
const UINT8 au8ZMSCL_UNICODE[]      = {"AT+ZMSGL=6,4\r"};

//¶ÌÐÅÉ¾³ý
const UINT8 au8CMGD[] = {"AT+CMGD="};
//·¢ËÍ¶ÌÏ¢
typedef struct _tsCMGS
{
  UINT8 *pCmd; //´æ·ÅATÃüÁî AT+CMGS=

  UINT8 *pPhoneNum;  //´æ·Åµç»°ºÅÂë£¬¼ÓÇ°ºóµÄ¡°¡±

  UINT8 * au8Info;    //´æ·ÅËù·¢µÄÐÅÏ¢Ö¸Õë

}tsCMGS, *pCMGS;

//º¯ÊýÉùÃ÷
void CdmaCmdInit(void);


/*****************************************************************************
 º¯ Êý Ãû  : CdmaSendMsg
 ¹¦ÄÜÃèÊö  : CDMAÄ£¿é·¢ËÍ¶ÌÐÅ

 ÊäÈë²ÎÊý  : none
 ·µ »Ø Öµ  : none,
*****************************************************************************/
BOOL CdmaSendMsg(const UINT8 * ptr, UINT8 u8Len)
{
//  Uart1EmptyRxBuf();
  Uart1SendBuffer(ptr, u8Len);
  return 1;
}

/*****************************************************************************
 º¯ Êý Ãû  : AssembleCdmaMsg
 ¹¦ÄÜÃèÊö  : AssembleCdmaMsg

 ÊäÈë²ÎÊý  : *pNum -- ·¢ËÍºÅÂëµÄÖ¸Õë
             *pBuf -- ·¢ËÍÐÅÏ¢Ö¸Õë

 ·µ »Ø Öµ  : TRUE, FALSE
*****************************************************************************/
const UINT8 au8CMGS[8] = {"AT+CMGS="};
BOOL AssembleCdmaMsg(UINT8 *pNum, UINT8 *pBuf, UINT8 u8Len)
{
  UINT8 *pMsg;
//  UINT8 *pRxBuf;
  UINT8 i=0;
//  SINT8 s8Dog;
//  UINT8 u8Back;

  pMsg = OSMemGet(pSmallMem, &err);
//  pRxBuf = OSMemGet(pSmallMem, &err);

  memcpy(&pMsg[i], &au8CMGS[0], 8);
  i += 8;
  pMsg[i++] = '"';
  memcpy(&pMsg[i], pNum, 11);
  i += 11;
  pMsg[i++] = '"';
  pMsg[i++] = '\r';

  memcpy(&pMsg[i], pBuf, u8Len);
  i += u8Len;

  pMsg[i++] = 0x1A;   //½áÊø·û

  CdmaSendMsg(pMsg, i);
  g_u8TimerDelay=0;
  while(g_u8TimerDelay < 10);    //ÑÓÊ±20ms

  OSMemPut(pSmallMem, pMsg);

  return TRUE;
 /*
  StopWatchDog();
  s8Dog = SwdGet();
  SwdSetLifeTime(s8Dog, 2000);
  SwdEnable(s8Dog);
  CdmaSendMsg(pMsg, i);

  while(TRUE)
  {
    if(Uart1GetRxDataLength() > 21)
    {
      Uart1GetRxData(&pRxBuf[0], 22);
      i=0;
      while((pRxBuf[i] != 'O') && (i<22))
      {
        i += 1;
      }
      OSMemPut(pSmallMem, pMsg);
      OSMemPut(pSmallMem, pRxBuf);
      SwdFree(s8Dog);

      return TRUE;
    }

    if(SwdGetLifeTime(s8Dog) == 0)
    {
      SwdReset(s8Dog);

      //·¢ËÍ10°üÊý¾Ý»¹Ã»ÓÐ½ÓÊÕµ½ÃüÁî£¬Ö±½Ó·µ»Ø
      ++u8Back;
      if(u8Back > 10)
      {
        OSMemPut(pSmallMem, pMsg);
        OSMemPut(pSmallMem, pRxBuf);
        SwdFree(s8Dog);

        return FALSE;
      }

      //Çå¿Õ´®¿Ú»º³åÇø
//      Uart1EmptyRxBuf();
      //·¢ËÍÄ£¿éÐÍºÅÃüÁî
      CdmaSendMsg(pMsg, i);
     }
  }*/
}

void CdmaToBeiDou(void)
{
  g_bCdmaSend = FALSE;
  g_bBdSend		= TRUE;
  CdmaPowerOff();

  BeiDouPowerOn();
  BdSendCmdICJC();
}

void BeiDouToCdma(void)
{
  g_bCdmaSend = TRUE;
  g_bBdSend	  = FALSE;

  CdmaPowerOn();
  StopWatchDog();           // ¹Ø±Õ¿´ÃÅ¹·
  CdmaInit();
  FreeWatchDog();          //ÊÍ·Å¿´ÃÅ¹·

  BeiDouPowerOff();
}


/*****************************************************************************
 º¯ Êý Ãû  : DeletCdmaMsg()
 ¹¦ÄÜÃèÊö  : ¶ÌÐÅÉ¾³ý¹¦ÄÜ

 ÊäÈë²ÎÊý  : u8Index -- ËùÒªÉ¾³ýµÄ¶ÌÐÅ±àºÅ, ×î¶à´æ´¢30Ìõ¶ÌÐÅ
                        0xff -- É¾³ýÈ«²¿µÄ¶ÌÏûÏ¢

 ·µ »Ø Öµ  : TRUE, FALSE
*****************************************************************************/
BOOL DeletCdmaMsg(SINT8 s8Index)
{
    UINT8   au8Buf[24];
    UINT8   i= 0;
    UINT8 * pMsg;
//    SINT8   s8Dog;
//    UINT8   u8Back = 0;
//   UINT8   au8RxBuf[16];

    if((s8Index > MSG_MAX_SIZE) || (s8Index < 0))
    {
        return FALSE;
    }

    memset(&au8Buf[0], 0, 24);
    pMsg = &au8Buf[0];

    memcpy(&pMsg[i], &au8CMGD[0], 8);
    i += 8;

    if(s8Index == MSG_MAX_SIZE)
    {
        pMsg[i++] = 0x31;
        pMsg[i++] = ',';
        pMsg[i++] = 0x34;
    }
    else
    {
        if(s8Index < 9)
        {
            pMsg[i++] = s8Index + '0';
        }
        else
        {
            pMsg[i++] = s8Index / 10 + '0';
            pMsg[i++] = s8Index % 10 + '0';
        }
    }
    pMsg[i++] = '\r';

    //·¢ËÍÄ£¿éÉ¾³ýÃüÁî
    CdmaSendMsg(pMsg, i);
    
    return TRUE;

    //µ±Í¬Ê±½ÓÊÕ´óÁ¿ÐÅÏ¢Ê±£¬µÈ´ýÊ±¼ä¹ý³¤£¬ÔòÖ±½ÓºöÂÔµô·´À¡ÐÅÏ¢¡£
/*
    StopWatchDog();
    s8Dog = SwdGet();
    SwdSetLifeTime(s8Dog, 5000);
    SwdEnable(s8Dog);
    memset(&au8RxBuf[0], 0, 16);

    while(TRUE)
    {
        if(Uart1GetRxDataLength() > 2)
        {
          Uart1GetRxData(&au8RxBuf[0], 2);

          if(0 == memcmp(&au8RxBuf[0], "OK", 2))
          {
            return TRUE;
          }
        }

        if(SwdGetLifeTime(s8Dog) == 0)
        {
          SwdReset(s8Dog);

          //·¢ËÍ5°üÊý¾Ý»¹Ã»ÓÐ½ÓÊÕµ½ÃüÁî£¬Ö±½Ó·µ»Ø
          ++u8Back;
          if(u8Back > 2)
          {
            SwdFree(s8Dog);

            return FALSE;
          }

           //·¢ËÍÄ£¿éÉ¾³ýÃüÁî
           CdmaSendMsg(pMsg, i);
        }
    }*/
}

/*****************************************************************************
 º¯ Êý Ãû  : CdmaInit
 ¹¦ÄÜÃèÊö  : CDMAÄ£¿é³õÊ¼»¯

 ÊäÈë²ÎÊý  : none
 ·µ »Ø Öµ  : none,
*****************************************************************************/
void CdmaInit(void)
{ 
  UINT8 au8Buf[16];
	
  CdmaResetDir |= CdmaReset;
  CdmaOnOffDir |= CdmaOnOff;

  //IC¿¨¼ì²â
//  CdmaIcCheckSetIn();
  //CDMA RIÖÐ¶Ï£¬ÏÂ½µÑØ´¥·¢ÖÐ¶Ï
  CdmaRiIesEdge();
  CdmaRiDir &= ~CdmaRi;

  //cdmaÄ£¿é¸´Î»
  SetCdmaReset(0);
  g_u16TimerDelay=0;
  while(g_u16TimerDelay < 2);    //µÍµçÆ½ÑÓÊ±20ms
  SetCdmaReset(1);
  //cdmaÄ£¿é¿ª»ú
  SetCdmaOnOff(1);
  g_u16TimerDelay=0;
  while(g_u16TimerDelay < 1);    //ÑÓÊ±10ms
  SetCdmaOnOff(0);
  g_u16TimerDelay=0;
  while(g_u16TimerDelay < 100);  //ÑÓÊ±2s
  SetCdmaOnOff(1);
  g_u16TimerDelay=0;
  while(g_u16TimerDelay < 100);  //ÑÓÊ±5s£¬Ê±¼äÌ«¶Ì»á¼ì²â²»µ½ÊÖ»ú¿¨

  //¼ì²âÒ»ÏÂÊÇ·ñ²åÈëIC¿¨
#if DTIC_DET_EN > 0
  if(!CdmaIcDet())
  {
    if(g_bDebug & CDMA_DEBUG)
    {
      Uart2SendString("#CDMA IC Card Not Found!\r\n ");
    }

    CdmaToBeiDou();
//    return;
  }
  else
#endif
  {
    /*
    if(GetCdmaCSQ() > 20)					//ÐÅºÅÇ¿¶È¿ÉÒÔ·¢ËÍÐÅÏ¢
    {
      g_bCdmaSend = TRUE;
      g_bBdSend   = FALSE;

      BeiDouPowerOff();

      CdmaSendMsg(au8ZMSGL_ASCII, sizeof(au8ZMSGL_ASCII));

      CdmaDtrSetLow();             //À­µÍDTR£¬½«CDMAÍË³ö´ý»ú¡£
      CdmaDtrSetOut();
      CdmaLowPowerEnter();

      CdmaIndicatorEnable();
    }
    else
    {
      CdmaToBeiDou();
    }   */

    g_bCdmaSend = TRUE;
    g_bBdSend   = FALSE;

    CdmaSendMsg(au8ZMSGL_ASCII, sizeof(au8ZMSGL_ASCII));
    CdmaDtrSetLow();             //À­µÍDTR£¬½«CDMAÍË³ö´ý»ú¡£
    CdmaDtrSetOut();
    CdmaLowPowerEnter();
 //   CdmaIndicatorEnable();

    CdmaSendMsg(&au8CMGF[0], sizeof(au8CMGF));

    //cdmaÄ£¿é½ÓÊÕÃüÁî½âÎö³õÊ¼»¯¡£
    CdmaCmdInit();

    //Çå³ý½ÓÊÕ»º³åÇøÄÚÈÝ
    Uart1EmptyRxBuf();

	//·¢ËÍ¶Á¶ÌÏûÏ¢ÁÐ±í£¬²é¿´Éè±¸´ý»ú»òÕß¹Ø»úÊ±µÄ¶ÌÐÅ
	memset(&au8Buf[0], 0, 16);
	memcpy(&au8Buf[0], &au8CMGL[0], 8);
	au8Buf[8]  = '"';
	au8Buf[9]  = 'A';
	au8Buf[10] = 'L';
	au8Buf[11] = 'L';
	au8Buf[12] = '"';
	au8Buf[13] = '\r';
	CdmaSendMsg(&au8Buf[0], 14);
  }
}

/*****************************************************************************
 º¯ Êý Ãû  : SetLocationNbr
 ¹¦ÄÜÃèÊö  : ÉèÖÃ±¾»úºÅÂë

 ÊäÈë²ÎÊý  : PTR
 ·µ »Ø Öµ  : TRUE , FALSE
*****************************************************************************/
BOOL SetLocationNbr(UINT8 *ptr)
{
  UINT8 au8SetCNUM[24];
  UINT8 i,u8Back;
  SINT8 sDog;

  i = 0;
  memcpy(&au8SetCNUM[i], "AT+CNUM=", 8);
  i += 8;

  memcpy(&au8SetCNUM[i], ptr, 11);
  i += 11;

  au8SetCNUM[i++] = '\r';

  //Çå¿Õ´®¿Ú»º³åÇø
//  Uart1EmptyRxBuf();
  //·¢ËÍÄ£¿éÐÍºÅÃüÁî
  CdmaSendMsg(au8SetCNUM, i);

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);
  StopWatchDog();

  while(TRUE)
  {
    if(Uart1GetRxDataLength() > 1)
    {
      if(Uart1GetByte() == 'O')
      {
        if(Uart1GetByte() == 'K')
        {
          SwdFree(sDog);
          return TRUE;
        }
      }
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //·¢ËÍ5°üÊý¾Ý»¹Ã»ÓÐ½ÓÊÕµ½ÃüÁî£¬Ö±½Ó·µ»Ø
      ++u8Back;
      if(u8Back > 5)
      {
        SwdFree(sDog);
        return FALSE;
      }

      //Çå¿Õ´®¿Ú»º³åÇø
      Uart1EmptyRxBuf();
      //·¢ËÍÄ£¿éÐÍºÅÃüÁî
      CdmaSendMsg(au8SetCNUM, i);
     }
  }

}

/*****************************************************************************
 º¯ Êý Ãû  : GetLocationNbr
 ¹¦ÄÜÃèÊö  : ÉèÖÃ±¾»úºÅÂë

 ÊäÈë²ÎÊý  : NONE
 ·µ »Ø Öµ  : TRUE , FALSE
*****************************************************************************/
BOOL GetLocationNbr(UINT8 *ptr)
{
  UINT8 au8GetCNUM[16];
  UINT8 au8Buf[48];
  UINT8 i,u8Back;
  SINT8 sDog;

  memcpy(&au8GetCNUM[0], "AT+CNUM?\r", 10);

  //Çå¿Õ´®¿Ú»º³åÇø
  Uart1EmptyRxBuf();
  //·¢ËÍÄ£¿éÐÍºÅÃüÁî
  CdmaSendMsg(au8GetCNUM, 10);

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);
  StopWatchDog();

  while(TRUE)
  {
    if(Uart1GetRxDataLength() > 34)
    {
      Uart1GetRxData(au8Buf, 34);

      i = 0;
      while((au8Buf[i] != ':') && (i<34))
      {
        i++;
      }

      i += 1;
      memcpy(ptr, &au8Buf[i], 11);

      SwdFree(sDog);
      return TRUE;
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //·¢ËÍ5°üÊý¾Ý»¹Ã»ÓÐ½ÓÊÕµ½ÃüÁî£¬Ö±½Ó·µ»Ø
      ++u8Back;
      if(u8Back > 5)
      {
        SwdFree(sDog);
        return FALSE;
      }

      //Çå¿Õ´®¿Ú»º³åÇø
      Uart1EmptyRxBuf();
      //·¢ËÍÄ£¿éÐÍºÅÃüÁî
      CdmaSendMsg(au8GetCNUM, i);
     }
  }
}

/*****************************************************************************
 º¯ Êý Ãû  : GetCdmaModule
 ¹¦ÄÜÃèÊö  : »ñÈ¡CDMAÄ£¿éÐÍºÅ

 ÊäÈë²ÎÊý  : none
 ·µ »Ø Öµ  : none,
*****************************************************************************/
void GetCdmaModule(void)
{
  UINT8 i,u8Back;
  SINT8 sDog;
  UINT8 *pRxBuf;

  pRxBuf = OSMemGet(pSmallMem, &err);
//  memset(pRxBuf, 0, BUF_SMALL_SIZE);

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);
  StopWatchDog();

  //Çå¿Õ´®¿Ú»º³åÇø
  Uart1EmptyRxBuf();
  //·¢ËÍÄ£¿éÐÍºÅÃüÁî
  CdmaSendMsg(au8GetModuleIndex, sizeof(au8GetModuleIndex));

  while(TRUE)
  {
    if(Uart1GetRxDataLength() > 19)
    {
      Uart1GetRxData(pRxBuf, 20);

      i = 0;
      while((pRxBuf[i] != '\r') && (i<20))
      {
        i++;
      }

      if(memcmp(&pRxBuf[i-6], "MC8332", 6) == 0)
      {
        Uart2SendString("#Detect ZTE CDMA Module. The Product Model Is MC8332!\r\n");
      }
      else
      {
        Uart2SendString("#CDMA Module To Find The Failure!\r\n");
      }

      break;
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);
      i = 0;

      //·¢ËÍ5°üÊý¾Ý»¹Ã»ÓÐ½ÓÊÕµ½ÃüÁî£¬Ö±½Ó·µ»Ø
      ++u8Back;
      if(u8Back > 5)
        break;

      //Çå¿Õ´®¿Ú»º³åÇø
      Uart1EmptyRxBuf();
      //·¢ËÍÄ£¿éÐÍºÅÃüÁî
      CdmaSendMsg(au8GetModuleIndex, sizeof(au8GetModuleIndex));
    }
  }

  //ÊÍ·ÅÄÚ´æ
  OSMemPut(pSmallMem, pRxBuf);
  //ÊÍ·ÅÈí¼þ¶¨Ê±Æ÷
  SwdFree(sDog);
}

/*****************************************************************************
 º¯ Êý Ãû  : GetCdmaCSQ
 ¹¦ÄÜÃèÊö  : »ñÈ¡CDMAÐÅºÅÇ¿¶È

 ÊäÈë²ÎÊý  : none
 ·µ »Ø Öµ  : ÐÅºÅÇ¿¶È,0~31
*****************************************************************************/
UINT8 GetCdmaCSQ(void)
{
  UINT8 i,u8Back;
  SINT8 sDog;
  UINT8 *pRxBuf;
  UINT8 u8Temp;

  pRxBuf = OSMemGet(pSmallMem, &err);
//  memset(pRxBuf, 0, BUF_SMALL_SIZE);

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);
  //Çå¿Õ´®¿Ú»º³åÇø
  Uart1EmptyRxBuf();
  //·¢ËÍÄ£¿éÐÍºÅÃüÁî
  CdmaSendMsg(au8CSQ, sizeof(au8CSQ));
  StopWatchDog();

  while(TRUE)
  {
    if(Uart1GetRxDataLength() > 24)
    {
      Uart1GetRxData(pRxBuf, 24);

      i = 0;
      while((pRxBuf[i] != ':') && (i<24))
      {
        i++;
      }

      if((pRxBuf[i+3] >= 0x30) && (pRxBuf[i+2] >= 0x30))
      {
        u8Temp = (pRxBuf[i+2]-0x30)*10 + (pRxBuf[i+3]-0x30);
        break;
      }
     }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //·¢ËÍ10°üÊý¾Ý»¹Ã»ÓÐ½ÓÊÕµ½ÃüÁî£¬Ö±½Ó·µ»Ø
      ++u8Back;
      if(u8Back > 5)
      {
        CdmaToBeiDou();
        u8Temp = 0xFF;
        break;
      }

      //Çå¿Õ´®¿Ú»º³åÇø
      Uart1EmptyRxBuf();
      //·¢ËÍÄ£¿éÐÍºÅÃüÁî
      CdmaSendMsg(au8CSQ, sizeof(au8CSQ));
    }
  }

  //ÊÍ·ÅÄÚ´æ
  OSMemPut(pSmallMem, pRxBuf);
  //ÊÍ·ÅÈí¼þ¶¨Ê±Æ÷
  SwdFree(sDog);

  return u8Temp;
}

/*****************************************************************************
 º¯ Êý Ãû  : GetCdmaTime
 ¹¦ÄÜÃèÊö  : »ñÈ¡CDMAÊ±¼ä, ÉèÖÃµ½RTC

 ÊäÈë²ÎÊý  : *pTime£¬*pu8Len
 ·µ »Ø Öµ  : none
*****************************************************************************/
void GetCdmaTime(UINT8 *pTime, UINT8 *pu8Len)
{
  UINT8 i,j,u8Back;
  SINT8 sDog;
  UINT8 *pRxBuf;

  pRxBuf = OSMemGet(pSmallMem, &err);
//  memset(pRxBuf, 0, BUF_SMALL_SIZE);

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);
  //Çå¿Õ´®¿Ú»º³åÇø
  Uart1EmptyRxBuf();
  //·¢ËÍÄ£¿éÐÍºÅÃüÁî
  CdmaSendMsg(au8CCLK, sizeof(au8CCLK));

  while(TRUE)
  {
    if(Uart1GetRxDataLength() > 48)
    {
      Uart1GetRxData(pRxBuf, 48);

      i = 0;
      while((pRxBuf[i] != '"') && (i<48))
      {
        i++;
      }

      j = ++i;
      while((pRxBuf[j] != '"') && (j<48))
      {
        j++;
      }

      break;
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //·¢ËÍ10°üÊý¾Ý»¹Ã»ÓÐ½ÓÊÕµ½ÃüÁî£¬Ö±½Ó·µ»Ø
      ++u8Back;
      if(u8Back > 5)
        break;

      //Çå¿Õ´®¿Ú»º³åÇø
      Uart1EmptyRxBuf();
      //·¢ËÍÄ£¿éÐÍºÅÃüÁî
      CdmaSendMsg(au8CCLK, sizeof(au8CCLK));
    }
  }
  //Ê±¼ä¿½±´£¬ºöÂÔmsÊý¾Ý
  *pu8Len = j-i;
  memcpy(pTime, &pRxBuf[i], j-i);
  //ÊÍ·ÅÄÚ´æ
  OSMemPut(pSmallMem, pRxBuf);
  //ÊÍ·ÅÈí¼þ¶¨Ê±Æ÷
  SwdFree(sDog);
}


/*****************************************************************************
 º¯ Êý Ãû  : SetCdmaTimeToRtc
 ¹¦ÄÜÃèÊö  : »ñÈ¡CDMAÊ±¼ä, ÉèÖÃµ½RTC

 ÊäÈë²ÎÊý  : none
 ·µ »Ø Öµ  : TRUE -- Ð£×¼³É¹¦£» FALSE -- Ð£×¼Ê§°Ü
*****************************************************************************/
BOOL SetCdmaTimeToRtc(void)
{
  UINT8 au8Time[32];
  UINT8 u8Len;
  UINT8 i;
  _tsTimeCb sTime;

  GetCdmaTime(&au8Time[0], &u8Len);

  i = 2;   //Äê·ÝÆ«ÒÆ
  if((au8Time[i] >= 0x30) && (au8Time[i] <= 0x39))
    sTime.Year = (au8Time[i] - 0x30)*10 + (au8Time[i+1] - 0x30);
  else
    return FALSE;

  i += 3;   //ÔÂ·ÝÆ«ÒÆ
  if((au8Time[i] >= 0x30) && (au8Time[i] <= 0x39))
    sTime.Month = (au8Time[i] - 0x30)*10 + (au8Time[i+1] - 0x30);
  else
    return FALSE;

  i += 3;   //ÈÕÆÚÆ«ÒÆ
  if((au8Time[i] >= 0x30) && (au8Time[i] <= 0x39))
    sTime.Date = (au8Time[i] - 0x30)*10 + (au8Time[i+1] - 0x30);
  else
    return FALSE;

  i += 3;   //Ê±Æ«ÒÆ
  if((au8Time[i] >= 0x30) && (au8Time[i] <= 0x39))
    sTime.Hour = (au8Time[i] - 0x30)*10 + (au8Time[i+1] - 0x30);
  else
    return FALSE;

  i += 3;   //·ÖÆ«ÒÆ
  if((au8Time[i] >= 0x30) && (au8Time[i] <= 0x39))
    sTime.Minute = (au8Time[i] - 0x30)*10 + (au8Time[i+1] - 0x30);
  else
    return FALSE;

  i += 3;   //ÃëÆ«ÒÆ
  if((au8Time[i] >= 0x30) && (au8Time[i] <= 0x39))
    sTime.Second = (au8Time[i] - 0x30)*10 + (au8Time[i+1] - 0x30);
  else
    return FALSE;

  RtcConverHour(1);
//  SetRtcInt1Out();
  SetTimeToRtc(&sTime);

  //´ò¿ªRTCÍâ²¿ÖÐ¶Ï
  SetRtcInt1Out();

  return TRUE;
}

/*****************************************************************************
 º¯ Êý Ãû  : CdmaLowPowerEnter
 ¹¦ÄÜÃèÊö  : CDMAÄ£¿é½øÈëµÍ¹¦ºÄÄ£¿é

 ÊäÈë²ÎÊý  : PTR
 ·µ »Ø Öµ  : TRUE , FALSE
*****************************************************************************/
void CdmaLowPowerEnter(void)
{
  UINT8 au8RxBuf[24];
  UINT8 i,u8Back=0;
  SINT8 sDog;

  memset(au8RxBuf, 0, 24);

  //ÉèÖÃCDMA 32kÉî¶ÈË¯Ãß
  CdmaSendMsg(&au8EnableZDSLEEP[0], sizeof(au8EnableZDSLEEP));
  g_u8TimerDelay=0;
  while(g_u8TimerDelay < 10);  //ÑÓÊ±100ms£¬µÈ´ý»Ø¸´ÃüÁî

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);

  while(TRUE)
  {
    if(Uart1GetRxDataLength() > 17)
    {
      Uart1GetRxData(au8RxBuf, 18);

      i = 0;
      while((au8RxBuf[i] != 'O') && (i<18))
      {
        i++;
      }

      break;
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //·¢ËÍ10°üÊý¾Ý»¹Ã»ÓÐ½ÓÊÕµ½ÃüÁî£¬Ö±½Ó·µ»Ø
      ++u8Back;
      if(u8Back > 5)
      {
        CdmaToBeiDou();
        //ÊÍ·ÅÈí¼þ¶¨Ê±Æ÷
        SwdFree(sDog);
        return;
      }

      //·¢ËÍÄ£¿éÐÍºÅÃüÁî
      CdmaSendMsg(&au8EnableZDSLEEP[0], sizeof(au8EnableZDSLEEP));
    }
  }

  memset(au8RxBuf, 0, 24);

  //ÔÊÐíDTR¹Ü½Å¿ØÖÆ½øÈë´ý»ú
  CdmaSendMsg(&au8EnableZDSLEEPDTR[0], sizeof(au8EnableZDSLEEPDTR));
  while(g_u8TimerDelay < 10);  //ÑÓÊ±100ms£¬µÈ´ý»Ø¸´ÃüÁî

   while(TRUE)
  {
    if(Uart1GetRxDataLength() > 19)
    {
      Uart1GetRxData(au8RxBuf, 20);

      i = 0;
      while((au8RxBuf[i] != 'O') && (i<20))
      {
        i++;
      }

      break;
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //·¢ËÍ10°üÊý¾Ý»¹Ã»ÓÐ½ÓÊÕµ½ÃüÁî£¬Ö±½Ó·µ»Ø
      ++u8Back;
      if(u8Back > 5)
      {
        CdmaToBeiDou();
        //ÊÍ·ÅÈí¼þ¶¨Ê±Æ÷
        SwdFree(sDog);
        return;
      }

      //·¢ËÍÄ£¿éÐÍºÅÃüÁî
      CdmaSendMsg(&au8EnableZDSLEEPDTR[0], sizeof(au8EnableZDSLEEPDTR));
    }
  }
  SwdFree(sDog);
}

/*****************************************************************************
 º¯ Êý Ãû  : CdmaLowPowerExit
 ¹¦ÄÜÃèÊö  : CDMAÄ£¿éÍË³öµÍ¹¦ºÄÄ£¿é

 ÊäÈë²ÎÊý  : PTR
 ·µ »Ø Öµ  : TRUE , FALSE
*****************************************************************************/
void CdmaLowPowerExit(void)
{
  UINT8 au8RxBuf[24];
  UINT8 i,u8Back=0;
  SINT8 sDog;

  memset(au8RxBuf, 0, 24);

  //½ûÖ¹CDMA 32kÉî¶ÈË¯Ãß
  CdmaSendMsg(&au8DisableZDSLEEP[0], sizeof(au8DisableZDSLEEP));
  g_u8TimerDelay=0;
  while(g_u8TimerDelay < 10);  //ÑÓÊ±100ms£¬µÈ´ý»Ø¸´ÃüÁî

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);

  while(TRUE)
  {
    if(Uart1GetRxDataLength() > 17)
    {
      Uart1GetRxData(au8RxBuf, 18);

      i = 0;
      while((au8RxBuf[i] != 'O') && (i<18))
      {
        i++;
      }

      break;
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //·¢ËÍ10°üÊý¾Ý»¹Ã»ÓÐ½ÓÊÕµ½ÃüÁî£¬Ö±½Ó·µ»Ø
      ++u8Back;
      if(u8Back > 5)
      {
        CdmaToBeiDou();
        //ÊÍ·ÅÈí¼þ¶¨Ê±Æ÷
        SwdFree(sDog);
        return;
      }

      //·¢ËÍÄ£¿éÐÍºÅÃüÁî
      CdmaSendMsg(&au8DisableZDSLEEP[0], sizeof(au8DisableZDSLEEP));
    }
  }

  memset(au8RxBuf, 0, 24);

  //½ûÖ¹DTR¹Ü½Å¿ØÖÆ½øÈë´ý»ú
  CdmaSendMsg(&au8DisableZDSLEEPDTR[0], sizeof(au8DisableZDSLEEPDTR));
  while(g_u8TimerDelay < 10);  //ÑÓÊ±100ms£¬µÈ´ý»Ø¸´ÃüÁî

   while(TRUE)
  {
    if(Uart1GetRxDataLength() > 19)
    {
      Uart1GetRxData(au8RxBuf, 20);

      i = 0;
      while((au8RxBuf[i] != 'O') && (i<20))
      {
        i++;
      }

      break;
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //·¢ËÍ10°üÊý¾Ý»¹Ã»ÓÐ½ÓÊÕµ½ÃüÁî£¬Ö±½Ó·µ»Ø
      ++u8Back;
      if(u8Back > 5)
      {
        CdmaToBeiDou();
        //ÊÍ·ÅÈí¼þ¶¨Ê±Æ÷
        SwdFree(sDog);
        return;
      }

      //·¢ËÍÄ£¿éÐÍºÅÃüÁî
      CdmaSendMsg(&au8DisableZDSLEEPDTR[0], sizeof(au8DisableZDSLEEPDTR));
    }
  }

  SwdFree(sDog);
}

/*****************************************************************************
 º¯ Êý Ãû  : CdmaWakeUpCfg
 ¹¦ÄÜÃèÊö  : CDMAÄ£¿éÍË³öË¯ÃßÄ£Ê½£¬²¢¶ÁÈ¡CDMAÐÅºÅ£¬ÐÅºÅÇ¿¶ÈµÍÓÚ16Ê±×Ô¶¯ÇÐ»»µ½
             ±±¶·Í¨Ñ¶Ä£¿éÉÏ

 ÊäÈë²ÎÊý  : PTR
 ·µ »Ø Öµ  : TRUE , FALSE
*****************************************************************************/
void CdmaWakeUpCfg(void)
{
  CdmaDtrSetLow();     //À­µÍDTR£¬½«CDMA»½ÐÑ¡£();

  g_u16TimerDelay=0;
  while(g_u16TimerDelay < 200);  //ÑÓÊ±Ò»ÏÂ£¬µÈ´ýÄ£¿éÎÈ¶¨

  if(GetCdmaCSQ() > 16)					//ÐÅºÅÇ¿¶È¿ÉÒÔ·¢ËÍÐÅÏ¢
  {
    g_bCdmaSend = TRUE;
    g_bBdSend   = FALSE;

     BeiDouPowerOff();
  }
  else
  {
    CdmaToBeiDou();
  }
}

/*****************************************************************************
 º¯ Êý Ãû  : CdmaCmdInit
 ¹¦ÄÜÃèÊö  : CDMAÄ£¿éÃüÁî½ÓÊÕ³õÊ¼»¯

 ÊäÈë²ÎÊý  : none
 ·µ »Ø Öµ  : none,
*****************************************************************************/
void CdmaCmdInit(void)
{
  sCdmaCmdCb.u16BufDataLen  = 0;
  sCdmaCmdCb.u8MsgEnvent    = 0;
  sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_HEADER;
  sCdmaCmdCb.pu8AssembleBuf = OSMemGet(pSmallMem, &err);

  //Get One soft timer
  sCdmaCmdCb.sCdmaDog       = SwdGet();
  SwdSetName(sCdmaCmdCb.sCdmaDog, "CdmaTimer");
  SwdSetLifeTime(sCdmaCmdCb.sCdmaDog, 500);

  sCdmaCmdCb.sCsqDog       = SwdGet();
  SwdSetName(sCdmaCmdCb.sCsqDog, "CSQTimer");
  SwdSetLifeTime(sCdmaCmdCb.sCsqDog, 40000);     //Ã¿45Ãë»ñÈ¡Ò»´ÎÐÅºÅÇ¿¶È
  SwdEnable(sCdmaCmdCb.sCsqDog);

  memset(&au8MsgUnRead[0], 0, MSG_MAX_SIZE);
  bMsgReady = FALSE;
}

/*****************************************************************************
 º¯ Êý Ãû  : CdmaCmdAssemble
 ¹¦ÄÜÃèÊö  : CDMA½ÓÊÕÃüÁî½âÎö

 ÊäÈë²ÎÊý  : ÎÞ
 ·µ »Ø Öµ  : ÎÞ , ÎÞ
*****************************************************************************/
void CdmaCmdAssemble(UINT8 u8InData)
{
    switch(sCdmaCmdCb.eAssembleState)
    {
        case E_CDMA_RECEIVING_HEADER:
        {
            if(u8InData == '+')
            {
               sCdmaCmdCb.pu8AssembleBuf[0] = u8InData;
               sCdmaCmdCb.u8Cmd             = 0;
               sCdmaCmdCb.u16BufDataLen     = 1;
               sCdmaCmdCb.eAssembleState    = E_CDMA_RECEIVING_CMD;
            }
/*            else
            {
                if((u8InData == 'S')  && (sCdmaCmdCb.u8MsgEnvent & 0x0C == 0x0C))
                {
                    sCdmaCmdCb.pu8AssembleBuf[0] = u8InData;
                    sCdmaCmdCb.u8Cmd             = u8InData;
                    sCdmaCmdCb.u16BufDataLen     = 1;
                    sCdmaCmdCb.eAssembleState    = E_CDMA_RECEIVING_SMS;
                }
            }*/

            break;
        }

        case E_CDMA_RECEIVING_CMD:
        {
            if((u8InData >= 'A') && (u8InData <= 'Z'))
            {
                sCdmaCmdCb.u8Cmd += u8InData;
                sCdmaCmdCb.pu8AssembleBuf[sCdmaCmdCb.u16BufDataLen++] = u8InData;
            }
            else
            {
                if(u8InData == ':')
                {
                    sCdmaCmdCb.pu8AssembleBuf[sCdmaCmdCb.u16BufDataLen++] = u8InData;
                    sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_BODY;
                }
                else
                {
                    sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_HEADER;
                    sCdmaCmdCb.u16BufDataLen  = 0;
                }
            }

            break;
        }

        case E_CDMA_RECEIVING_BODY:
        {
            if(u8InData != '+')
            {
                //½«Êý¾Ý·ÅÈë×é°ü»º³åÇø
                sCdmaCmdCb.pu8AssembleBuf[sCdmaCmdCb.u16BufDataLen++] = u8InData;

                if((u8InData == 'K') && (sCdmaCmdCb.pu8AssembleBuf[sCdmaCmdCb.u16BufDataLen-2] == 'O'))
                {
                    sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_VALID;
                }
                else
                {
                  if((sCdmaCmdCb.u8Cmd == CMD_ZCEND) && (u8InData == 0x0d))
                  {
                      sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_VALID;
                  }
                }
            }
            else
            {
                if(u8InData == '+')
                {
                    Uart1InsertByte('+');
                    sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_VALID;
                }
            }
            break;
        }
/*
        case E_CDMA_RECEIVING_SMS:
        {
            sCdmaCmdCb.pu8AssembleBuf[sCdmaCmdCb.u16BufDataLen++] = u8InData;

            if((u8InData >= 'A') && (u8InData <= 'Z'))
            {
                sCdmaCmdCb.u8Cmd += u8InData;
            }

            if(u8InData == 0x0d)
            {
                sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_VALID;
            }

            break;
        }*/

        case E_CDMA_RECEIVING_VALID:
        {
            break;
        }

        default:
        {
            sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_HEADER;
            sCdmaCmdCb.u16BufDataLen  = 0;
            break;
        }
    }
}

/*****************************************************************************
 º¯ Êý Ãû  : RegisteUnReadMsg
 ¹¦ÄÜÃèÊö  : ±êÊ¶Î´¶Á¶ÌÐÅµ½×¢²á»º³åÇø

 ÊäÈë²ÎÊý  : ÎÞ
 ·µ »Ø Öµ  : BOOL
*****************************************************************************/
BOOL RegisteUnReadMsg(void)
{
    UINT8 *pRxBuf;
    UINT8  i = 0;
    UINT8  u8Temp = 0;

    //»ñÈ¡¿ÉÓÃµÄÄÚ´æµ¥Ôª
    pRxBuf = OSMemGet(pSmallMem, &err);
    memcpy(pRxBuf, sCdmaCmdCb.pu8AssembleBuf, sCdmaCmdCb.u16BufDataLen);

    while((pRxBuf[i] != '"') && (i < sCdmaCmdCb.u16BufDataLen))
    {
        i++;
    }

    if((pRxBuf[i+1] == 'M') && (pRxBuf[i+2] == 'T'))
    {
        if((pRxBuf[i+6] >= '0') && (pRxBuf[i+6] <= '9'))
        {
            if((pRxBuf[i+5] >= '0') && (pRxBuf[i+5] <= '9'))
            {
                u8Temp  = (pRxBuf[i+5] - '0') * 10;
                u8Temp += (pRxBuf[i+6] - '0');
            }
            else
            {
                u8Temp = 0xff;
            }
        }
        else
        {
            if((pRxBuf[i+5] >= '0') && (pRxBuf[i+5] <= '9'))
            {
                u8Temp  = (pRxBuf[i+5] - '0');
            }
            else
            {
                u8Temp = 0xff;
            }
        }
    }
    else
    {
        u8Temp = 0xff;
    }

    //ÊÍ·ÅÄÚ´æ
    OSMemPut(pSmallMem, pRxBuf);

    if(u8Temp < MSG_MAX_SIZE)
    {
        au8MsgUnRead[u8Temp] = 1;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/******************************************************************************
 * º¯ÊýÃû³Æ£ºGetUnReadMsg
 * ÃèÊö£º    µÃµ½Ò»ÌõÎ´¶ÁµÄ¶ÌÏûÏ¢±àºÅ
 *
 * ²ÎÊý£ºÎÞ
 * ·µ»Ø£ºÎ´¶Á¶ÌÐÅ±àºÅ
 *****************************************************************************/
SINT8 GetUnReadMsg(void)
{
  SINT8 i;

  for(i=0; i<MSG_MAX_SIZE; i++)
  {
    if(au8MsgUnRead[i] == 1)
    {
      au8MsgUnRead[i] = 0;
      return i;
    }
  }

  return -1;
}

/*****************************************************************************
 º¯ Êý Ãû  : GetCdmaCsqValue
 ¹¦ÄÜÃèÊö  : ´Ó×é°ü»º³åÇøÖÐ»ñÈ¡CDMAÐÅºÅÇ¿¶ÈÖµ

 ÊäÈë²ÎÊý  : none
 ·µ »Ø Öµ  : ÐÅºÅÇ¿¶È,0~31
*****************************************************************************/
SINT8 GetCdmaCsqValue(void)
{
    SINT8 s8Value=-1;   //CSQÖµ
    UINT8 au8Buf[24];
    UINT8 i=0;

    //»ñÈ¡»º³åÇøÖÐµÄÃüÁî
    memcpy(&au8Buf[0], sCdmaCmdCb.pu8AssembleBuf, sCdmaCmdCb.u16BufDataLen);

    while((au8Buf[i] != ':') && (i < sCdmaCmdCb.u16BufDataLen))
    {
        i++;
    }

    if((au8Buf[i+1] == ' ') && (au8Buf[i+2] >= '0') && (au8Buf[i+2] <= '9'))
    {
        i += 2;
        s8Value  = (au8Buf[i] - '0') * 10;
        s8Value += (au8Buf[i+1] - '0');

        if(s8Value > 31)
        {
            return -1;
        }
        else
        {
            return s8Value;
        }
    }
    else
    {
        return -1;
    }

}

/*****************************************************************************
 º¯ Êý Ãû  : SendReadMsgCmd
 ¹¦ÄÜÃèÊö  : ÏòCDMAÄ£¿é·¢ËÍ¶ÁÈ¡¶ÌÐÅÃüÁî

 ÊäÈë²ÎÊý  : ÎÞ
 ·µ »Ø Öµ  : BOOL
*****************************************************************************/
SINT8 SendReadMsgCmd(void)
{
    UINT8 u8Buf[24];
    UINT8 i=0;
    SINT8 s8MsgIndex = -1;

    s8MsgIndex = GetUnReadMsg();
    if(s8MsgIndex < 0)
    {
        return -1;
    }

    s8CurrentIndex = s8MsgIndex;
    memset(&u8Buf[0], 0, 24);
    memcpy(&u8Buf[0], &au8CMGR[0], sizeof(au8CMGR));
    i = sizeof(au8CMGD)-1;

    if(s8MsgIndex > 9)
    {
        u8Buf[i]  = s8MsgIndex / 10;
        u8Buf[i] += '0';
        i++;
        u8Buf[i]  = s8MsgIndex % 10;
        u8Buf[i] += '0';
        i++;
        u8Buf[i++] = '\r';

        CdmaSendMsg(&u8Buf[0], i);
    }
    else
    {
        u8Buf[i]  = s8MsgIndex + '0';
        i++;
        u8Buf[i++] = '\r';

        CdmaSendMsg(&u8Buf[0], i);
    }

    return s8MsgIndex;
}

/*****************************************************************************
 º¯ Êý Ãû  : ReadCdmaMsg
 ¹¦ÄÜÃèÊö  : ¶ÁCDMA¶ÌÐÅ

 ÊäÈë²ÎÊý  : ÎÞ
 ·µ »Ø Öµ  : ÎÞ , ÎÞ
*****************************************************************************/
void ReadCdmaMsg(void)
{
	UINT8 au8Buf[16];
	
	//·¢ËÍ¶Á¶ÌÏûÏ¢ÁÐ±í£¬²é¿´Éè±¸´ý»ú»òÕß¹Ø»úÊ±µÄ¶ÌÐÅ
	memset(&au8Buf[0], 0, 16);
	memcpy(&au8Buf[0], &au8CMGL[0], 8);
	au8Buf[8]  = '"';
	au8Buf[9]  = 'A';
	au8Buf[10] = 'L';
	au8Buf[11] = 'L';
	au8Buf[12] = '"';
	au8Buf[13] = '\r';
	CdmaSendMsg(&au8Buf[0], 14);
}


/*****************************************************************************
 º¯ Êý Ãû  : CdmaCmdHandler
 ¹¦ÄÜÃèÊö  : CDMA½ÓÊÕÃüÁî½âÎö

 ÊäÈë²ÎÊý  : ÎÞ
 ·µ »Ø Öµ  : ÎÞ , ÎÞ
*****************************************************************************/
void CdmaCmdHandler(UINT8 u8Cmd)
{
  SINT8 Csq = -1;

    switch(u8Cmd)
    {
        case CMD_CSQ:
        {
            if(g_bDebug & CDMA_DEBUG)    //´òÓ¡µ÷ÊÔÐÅÏ¢
            {
                Uart2SendString(" CDMA CSQ Parse Successful!\r\n");
            }

            Csq = GetCdmaCsqValue();
            //ÐÅºÅÇ¿¶ÈÐ¡ÓÚ20£¬ÔòÇÐ»»µ½±±¶·Ä£Ê½
            //ÔÙ´Î¼ì²âÔòÐèÔÚÏÂ´ÎÏµÍ³»½ÐÑºó¡£
            if(Csq < 20)
            {
                CdmaToBeiDou();
            }

            break;
        }

        case CMD_ZCANS:
        {
          // ¹Ø±ÕÐÅºÅÇ¿¶È»ñÈ¡¶¨Ê±Æ÷£¬·ÀÖ¹ÔÚ½ÓÊÕÐÅÏ¢Ê±·¢ËÍÐÅºÅ»ñÈ¡µ¼ÖÂ»ñÈ¡Ê§°Ü
            SwdDisable(sCdmaCmdCb.sCsqDog);
            sCdmaCmdCb.u8MsgEnvent |= BIT0;
            if(g_bDebug & CDMA_DEBUG)    //´òÓ¡µ÷ÊÔÐÅÏ¢
            {
                Uart2SendString(" CDMA ZCANS Parse Successful!\r\n");
            }
            break;
        }

        case CMD_ZCCNT:
        {
            SwdDisable(sCdmaCmdCb.sCsqDog);
            sCdmaCmdCb.u8MsgEnvent |= BIT1;
            if(g_bDebug & CDMA_DEBUG)    //´òÓ¡µ÷ÊÔÐÅÏ¢
            {
                Uart2SendString(" CDMA ZCCNT Parse Successful!\r\n");
            }
            break;
        }

        case CMD_CMTI:
        {
            SwdDisable(sCdmaCmdCb.sCsqDog);
            RegisteUnReadMsg();
            if(g_bDebug & CDMA_DEBUG)    //´òÓ¡µ÷ÊÔÐÅÏ¢
            {
                Uart2SendString(" CDMA CMTI Parse Successful!\r\n");
            }
            break;
        }

        case CMD_ZCEND:
        {
            SwdDisable(sCdmaCmdCb.sCsqDog);
            sCdmaCmdCb.u8MsgEnvent |= BIT3;
            if(g_bDebug & CDMA_DEBUG)    //´òÓ¡µ÷ÊÔÐÅÏ¢
            {
                Uart2SendString(" CDMA ZCEND Parse Successful!\r\n");
            }
            break;
        }

        case CMD_SMS:
        {
            SwdDisable(sCdmaCmdCb.sCsqDog);
            sCdmaCmdCb.u8MsgEnvent = 0;
            if(g_bDebug & CDMA_DEBUG)    //´òÓ¡µ÷ÊÔÐÅÏ¢
            {
                Uart2SendString(" CDMA SMS READY!\r\n");
            }
            break;
        }

      case CMD_CMGR:
      {
          SwdEnable(sCdmaCmdCb.sCsqDog);

          //½âÎöÒ»ÌõÐÅÏ¢É¾³ýÒ»Ìõ¡
          ReadCdmaMsg();
          DeletCdmaMsg(s8CurrentIndex);
          s8CurrentIndex = -1;
          if(g_bDebug & CDMA_DEBUG)    //´òÓ¡µ÷ÊÔÐÅÏ¢
          {
            Uart2SendString(" CDMA CMGR Parse Successful!\r\n");
          }
          break;
      }

	  case CMD_CMGL:
	  {	
		  if(g_bDebug & CDMA_DEBUG)    //´òÓ¡µ÷ÊÔÐÅÏ¢
          {
            Uart2SendString(" CDMA CMGL Parse Successful!\r\n");
          }
		  
          break;
	  }

        default: break;
    }
}

/*****************************************************************************
 º¯ Êý Ãû  : CdmaCmdRun
 ¹¦ÄÜÃèÊö  : CDMAÊý¾Ý½âÎö

 ÊäÈë²ÎÊý  : ÎÞ
 ·µ »Ø Öµ  : ÎÞ , ÎÞ
*****************************************************************************/
void CdmaCmdRun(void)
{
  if(g_bCdmaSend == TRUE)
  {
    //´Ó´®¿Ú»º³åÇøÄÚ»ñÈ¡Êý¾Ý£¬¶ÔÆä½øÐÐ½âÎö
    //Ö÷ÒªÕë¶Ô¶ÌÐÅ½ÓÊÕÃüÁî¹ý³ÌºÍCSQÐÅºÅÇ¿¶ÈµÄ½ÓÊÕ¡£
    while((sCdmaCmdCb.eAssembleState != E_CDMA_RECEIVING_VALID) && (Uart1GetRxDataLength() > 0))
    {
        CdmaCmdAssemble(Uart1GetByte());

        if(SwdIsEnable(sCdmaCmdCb.sCdmaDog) == TRUE)
        {
            SwdDisable(sCdmaCmdCb.sCdmaDog);
        }
    }

    //»ñµÃÓÐÐ§µÄÃüÁî°ü£¬Ôò½øÐÐÃüÁî½âÎö
    if(sCdmaCmdCb.eAssembleState == E_CDMA_RECEIVING_VALID)
    {
        if(CMD_ASSERT(sCdmaCmdCb.u8Cmd))
        {
            CdmaCmdHandler(sCdmaCmdCb.u8Cmd);

            if(g_bDebug & CDMA_DEBUG)    //´òÓ¡µ÷ÊÔÐÅÏ¢
            {
               Uart2SendBuffer(sCdmaCmdCb.pu8AssembleBuf, sCdmaCmdCb.u16BufDataLen);
            }
         }

        memset(sCdmaCmdCb.pu8AssembleBuf, 0, sCdmaCmdCb.u16BufDataLen);
        sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_HEADER;
        sCdmaCmdCb.u16BufDataLen = 0;
    }
    else
    {
       if(SwdIsEnable(sCdmaCmdCb.sCdmaDog) == TRUE)
       {
           if(SwdGetLifeTime(sCdmaCmdCb.sCdmaDog) == 0)
           {
                sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_HEADER;
                sCdmaCmdCb.u16BufDataLen  = 0;

                SwdReset(sCdmaCmdCb.sCdmaDog);
                SwdDisable(sCdmaCmdCb.sCdmaDog);
           }
       }
       else
       {
           if(sCdmaCmdCb.u16BufDataLen != 0)
           {
                SwdReset(sCdmaCmdCb.sCdmaDog);
                SwdEnable(sCdmaCmdCb.sCdmaDog);
           }
       }
    }

    if(SwdGetLifeTime(sCdmaCmdCb.sCsqDog) == 0)
    {
        SwdReset(sCdmaCmdCb.sCsqDog);

		ReadCdmaMsg();

 //       CdmaSendMsg(au8CSQ, sizeof(au8CSQ));
    }
	else
	{
    	SendReadMsgCmd();
	}
  }
}

