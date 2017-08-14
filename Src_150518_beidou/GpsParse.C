/******************************************************************************
*											
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved										
*
* 文 件 名： GpsParse.C
* 描    述：GPS数据解析

* 创建日期： 2014年9月5日11:31:59
* 作    者： Bob
* 当前版本： V1.00

------------------------------------------------------
*******************************************************************************/

#define   GPS_GLOBALS
#include "GpsParse.h"
#include "string.h"
#include "ctype.h"
#include "global.h"
#include "Timer.h"
#include "Uart2.h"
#include "Uart3.h"
#include "SystemCtl.h"
#include "OS_MEM.h"
#include "SoftTimer.h"
#include "Rtc.h"
#include "Communication.h"

#define GPS_BUF_SIZE    BUF_LARGE_SIZE

#define TEST


//GPS 所需信息结构声明
typedef struct
{
  //定位有效标志
  BOOL bActive;

  // UTC时间
//  UINT8 UTC_Time[6];
//  UINT8 UTC_Date[6];

  // 纬度
  UINT8 GPS_Y[10];
  //纬度半球
  UINT8 NorthSouth;
  
  // 经度
  UINT8 GPS_X[11];
  // 经度半球
  UINT8 EastWest;

  //地面速率（000.0 ~999.9）   ******************************************/
//  UINT8 Speed[8];

  //地面航向（000.0~359.9， 以真北方向为基准）
//  UINT8 Course[8];

  //使用卫星数量
  UINT8 UseEphSum;

}_tsGpsInfo;


#define GPS_MAX_SIZE      8
//外部结构声明，主要用来接收缓冲器的数据
GPS_EXT _tsGpsInfo sGpsInfoBuf[GPS_MAX_SIZE];


//数据包解析协议
typedef enum
{
  //等待数据的起始标志
  E_UART_CMD_RECEIVING_HEADER,
  //接收数据
  E_UART_CMD_RECEIVING_BODY,
  //数据有效
  E_UART_CMD_RECEIVING_VALID
		
}teUartCmdAssembleState;

/* 数据组包结构声明 */
typedef struct
{
  UINT8 *pu8AssembleBuf;  //组包区数据指针
  
  UINT8 *pu8CmdIndex;     //命令索引指针

//  BOOL bGpsParse;         //数据解析标志，TRUE--正在解析，FALSE -- 解析完毕 
		
  UINT16 u16BufDataLen ;  //组包的数据长度
	
  teUartCmdAssembleState eAssembleState;  //组包状态
  
  SINT8 sGpsDog;
  
  UINT8 u8GpsRxNum;       //已经接收了有效GPS信息的包数
	
}_tsUartCmdCb;

_tsUartCmdCb sGpsCmdCb;

//GPS命令类型
const UINT8 *pGpsCmd[] = {"$GNGGA", "$GNGLL","$GNRMC", "$GNVTG"};
//GPS 命令类型
enum eNmeaType
{
  GNNON = 0x00,
  GNGGA = 0x01,
  GNGLL = 0x02,
  GNRMC = 0x04,
  GNVTG = 0x08
};

#define GPS_RX_INVALID      0
#define GPS_RX_VALID        1
#define GPS_RX_COMPLETE     2

//gps module gngga cmd index
#define GPS_UTC_TIME        1
#define GPS_LOC_Y           2
#define GPS_N_S             3
#define GPS_LOC_X           4
#define GPS_E_W             5
#define GPS_VALID           6
#define GPS_STAR            7
#define GPS_CHECK           13

//一年内月份列表
const UINT8 au8Leap[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const UINT8 au8Year[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#define RtcHex2Bcd(u8Hex) ((((u8Hex % 100) / 10) << 4) + ((u8Hex % 100) % 10))
#define Bcd2Hex(u8Bcd) ((((u8Bcd & 0xf0) >> 4) % 10) * 10 + ((u8Bcd & 0x0f) % 10))

UINT8 u8Num=0;
BOOL bRtcFlag=FALSE;


/**********************************************************************************************
 函数名称：NmeaPackType
 函数功能：GPS 命令解析

 参数： 无
 返回： 0183协议命令类型
**********************************************************************************************/
SINT8 NmeaPackType(const UINT8 *pu8Buf, UINT8 u8Size)
{
  if(u8Size < 6)
    return GNNON;
  else if(0 == memcmp(pu8Buf, pGpsCmd[0], 6))
    return GNGGA;
  else if(0 == memcmp(pu8Buf, pGpsCmd[1], 6))
    return GNGLL;
  else if(0 == memcmp(pu8Buf, pGpsCmd[2], 6))
    return GNRMC;

  return GNNON;
}

/**********************************************************************************************
 函数名称：CreatCmdIndex
 函数功能：建立命令索引

 参数： 无
 返回： 无
**********************************************************************************************/
void CreatCmdIndex(void)
{
  UINT8 i,j=0;
  
  memset(sGpsCmdCb.pu8CmdIndex, 0, BUF_SMALL_SIZE);
  
  //提取的指令建立索引表
  for(i=0; i<sGpsCmdCb.u16BufDataLen; i++)
  {
     if(sGpsCmdCb.pu8AssembleBuf[i] == ',')
     {
       sGpsCmdCb.pu8CmdIndex[j++] = i;
     }
   }
}

/***********************************************************************************
  函数名称：RealCmdProcess
  函数功能：查找命令数据第N个参数的偏移

  输入：需要获取的参数位置编码
  输出：返回第N个","之后的信息，需要*buffer有效并创建索引后才可以执行
***********************************************************************************/
UINT8* RealCmdProcess(UINT8 num )
{
  if ( num < 1 )
    return  &sGpsCmdCb.pu8AssembleBuf[0];
  return  &sGpsCmdCb.pu8AssembleBuf[ sGpsCmdCb.pu8CmdIndex[num - 1] + 1];
}


/**********************************************************************************************
 函数名称：GpsInfoProcess
 函数功能：处理GPS有效命令

 参数： GPS命令代码
 返回： 无
**********************************************************************************************/
UINT8 GpsInfoProcess(UINT8 u8Cmd)
{
  UINT8 * str;
  
  //GPGGA 指令解析
  if(u8Cmd == GNGGA )
  {
    if( *RealCmdProcess(GPS_VALID) != '0')  //第6个参数为定位有效位
    {
      sGpsInfoBuf[u8Num].bActive = GPS_RX_VALID;
    }
    else
    {
      if(g_bDebug & GPS_DEBUG)
      {
        Uart3SendString("#Gps Receiving Invalid Data!\r\n");
      }
      
      return GPS_RX_INVALID;
    }
    
    //定位有效时获取经纬度信息
    if(sGpsInfoBuf[u8Num].bActive == GPS_RX_VALID)
    {
 //     str = RealCmdProcess(GPS_UTC_TIME);                             //第1个参数为时间
 //     memcpy(pGpsInfoBuf[u8Num]->GPS_Y, str, 10);

      str = RealCmdProcess(GPS_LOC_Y);                                  //第2个参数为维度
      memcpy(sGpsInfoBuf[u8Num].GPS_Y, str, 10);
//      sGpsInfoBuf[u8Num].GPS_Y[10] = 0;

      sGpsInfoBuf[u8Num].NorthSouth = *RealCmdProcess(GPS_N_S);         //第3个参数为南北半球

      str = RealCmdProcess(GPS_LOC_X);                                  //第4个参数为经度
      memcpy(sGpsInfoBuf[u8Num].GPS_X, str, 11);
 //     sGpsInfoBuf[u8Num].GPS_X[11] = 0;

      sGpsInfoBuf[u8Num].EastWest = *RealCmdProcess(GPS_E_W);           //第5个参数为东西半球

      sGpsInfoBuf[u8Num].UseEphSum = (UINT8)(atof(RealCmdProcess(GPS_STAR)));    //GPGGA第7个参数为卫星个数
                
      if(g_bDebug & GPS_DEBUG)
      {
        Uart3SendString("#========================================================================#\r\n");
        Uart3SendString("#The Current Position is: 纬度 <");
        Uart3SendBuffer(sGpsInfoBuf[u8Num].GPS_Y, 11);
        Uart3SendString(" >...经度 < ");
        Uart3SendBuffer(sGpsInfoBuf[u8Num].GPS_X, 12);
        Uart3SendString(" >....#\r\n");
        Uart3SendString("#========================================================================#\r\n");
      }
    
      u8Num++;
      sGpsCmdCb.u8GpsRxNum++;
      
      if(u8Num == GPS_MAX_SIZE)
      {
        u8Num = 0;
        sGpsCmdCb.u8GpsRxNum = 0;
        if(g_bDebug & GPS_DEBUG)
        {
          Uart3SendString("#Gps Receiving Data Complete!\r\n");
        }
        
        return GPS_RX_COMPLETE;
      }
    }
    
    return GPS_RX_VALID;
  }
  
  return GPS_RX_VALID;
}

/**********************************************************************************************
 函数名称：StringToUint32
 函数功能：

 参数： 无
 返回： UINT32
**********************************************************************************************/
UINT32 StringToUint32(UINT8 *ptr)
{
  UINT32 u32Temp=0;
  
  u32Temp += (UINT32)(ptr[0] - '0') * 10000;
  u32Temp += (UINT32)(ptr[1] - '0') * 1000;
  u32Temp += (UINT32)(ptr[2] - '0') * 100;
  u32Temp += (UINT32)(ptr[3] - '0') * 10;
  u32Temp += (UINT32)(ptr[4] - '0');
  
  return u32Temp;
}

/**********************************************************************************************
 函数名称：Uint32ToString
 函数功能：

 参数： UINT32
 返回： *ptr
**********************************************************************************************/
void Uint32ToString(UINT8 *ptr, UINT32 u32Temp)
{
  ptr[0] = u32Temp / 10000 + '0';
  ptr[1] = u32Temp % 10000 / 1000 + '0';
  ptr[2] = u32Temp % 1000 /100 + '0';
  ptr[3] = u32Temp %100 /10 + '0';
  ptr[4] = u32Temp % 10 + '0';
}

/**********************************************************************************************
 函数名称：CalibratLocation
 函数功能：校准GPS数据，取平均值作为最后的定位经纬度

 参数： 无
 返回： 无
**********************************************************************************************/
void CalibratLocation(void)
{
  UINT8 i=0;
  UINT32 u32X=0;
  UINT32 u32Y=0;
  UINT8 au8Buf[5];
  
  if(sGpsCmdCb.u8GpsRxNum != 0)
  {
    for(i=0; i<sGpsCmdCb.u8GpsRxNum; i++)
    {
      u32Y += StringToUint32(&sGpsInfoBuf[i].GPS_Y[5]);
    }
    u32Y = u32Y / sGpsCmdCb.u8GpsRxNum;
    Uint32ToString(au8Buf, u32Y);
    memcpy(&sGpsInfoBuf[GPS_MAX_SIZE-1].GPS_Y[5], au8Buf, 5);
    
    for(i=0; i<sGpsCmdCb.u8GpsRxNum; i++)
    {
      u32X += StringToUint32(&sGpsInfoBuf[i].GPS_X[6]);
    }
    u32X = u32X / sGpsCmdCb.u8GpsRxNum;
    Uint32ToString(au8Buf, u32X);
    memcpy(&sGpsInfoBuf[GPS_MAX_SIZE-1].GPS_X[6], au8Buf, 5);
  }
  
  //将有效数据存放到最后位置上
  sSendInfo.pGPS_Y = sGpsInfoBuf[GPS_MAX_SIZE-1].GPS_Y;
  sSendInfo.pGPS_X = sGpsInfoBuf[GPS_MAX_SIZE-1].GPS_X;
}


/**********************************************************************************************
 函数名称：CalibratRtc
 函数功能：校准RTC

 参数： u8Cmd
 返回： 无
**********************************************************************************************/
BOOL CalibratRtc(UINT8 u8Cmd)
{
  UINT8 *ptr;
  UINT8 au8Time[6];
  UINT8 au8Date[6];
  _tsTimeCb sTime;
  UINT16 Year;
  UINT8 Temp;
  
  //GNRMC 指令解析
  if(u8Cmd == GNRMC )
  {
    if( *RealCmdProcess(2) == 'A')  //第2个参数为数据有效位
    {
      ptr = RealCmdProcess(1);           //utc时间
      memcpy(au8Time, ptr, 6);
      
      ptr = RealCmdProcess(9);           //utc日期
      memcpy(au8Date, ptr, 6);
      
      //string to time
      sTime.Hour   = (au8Time[0] - 0x30)*10 + (au8Time[1] - 0x30) + 8;
      sTime.Minute = (au8Time[2] - 0x30)*10 + (au8Time[3] - 0x30);
      sTime.Second = (au8Time[4] - 0x30)*10 + (au8Time[5] - 0x30);

      sTime.Date  = (au8Date[0] - 0x30)*10 + (au8Date[1] - 0x30);
      sTime.Month = (au8Date[2] - 0x30)*10 + (au8Date[3] - 0x30);
      sTime.Year  = (au8Date[4] - 0x30)*10 + (au8Date[5] - 0x30);
  
      //UTC时间=>北京时间转换
      if(sTime.Hour > 23)
      {
        sTime.Hour -= 24;
        sTime.Date += 1;

        Year = sTime.Year + 2000;
        if(((Year%4 == 0) && (Year%100!=0)) || (Year%400 == 0))
        {
          Temp = au8Leap[sTime.Month];
        }
        else
        {
          Temp = au8Year[sTime.Month];
        }

        if(Temp < sTime.Date)
        {
          sTime.Date = 1;
          sTime.Month += 1;
          if(sTime.Month >12 )
          {
            sTime.Month = 1;
            sTime.Year += 1;
          }
        }
      }
      
      //set time to rtc
      RtcConverHour(1);
//      SetRtcInt1Out();
      SetTimeToRtc(&sTime);
      
      //打开RTC外部中断
      SetRtcInt1Out();
      
      return TRUE;
    }
    
    return FALSE;
  }
  
  return FALSE;
}

/***********************************************************************************************
  函数名称：GpsCmdAssemble(UINT8 u8InData)
  函数功能：把接收的数据组合成有效的数据包

  输入：u8InData
  输出：无
***********************************************************************************************/
void GpsCmdAssemble(UINT8 u8InData)
{
  switch(sGpsCmdCb.eAssembleState)
  {
    case E_UART_CMD_RECEIVING_HEADER:
    {
      if(u8InData == '$')
      {
        sGpsCmdCb.pu8AssembleBuf[0] = u8InData;
        sGpsCmdCb.u16BufDataLen = 1;
	      sGpsCmdCb.eAssembleState = E_UART_CMD_RECEIVING_BODY;	
      }
      break;
     }	
		
    case E_UART_CMD_RECEIVING_BODY:
    {
      sGpsCmdCb.pu8AssembleBuf[sGpsCmdCb.u16BufDataLen++] = u8InData;
      if(u8InData == '\r')
      {
        sGpsCmdCb.eAssembleState = E_UART_CMD_RECEIVING_VALID;
      }
      else
      {
        if(sGpsCmdCb.u16BufDataLen == GPS_BUF_SIZE)	
        {
          sGpsCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
          sGpsCmdCb.u16BufDataLen = 0;
          break;
         }
        }
        break;
      }
		
    case E_UART_CMD_RECEIVING_VALID:
    { 
      break;
    }
		
    default:
    {
      sGpsCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
      sGpsCmdCb.u16BufDataLen = 0;
    }
  }	
}

/**********************************************************************************************
 函数名称：GpsCmdInit
 函数功能：GPS cmd parse init

 参数： 无
 返回： 无
**********************************************************************************************/
void GpsCmdInit(void)
{
  //  sGpsCmdCb.bGpsParse = FALSE;
  sGpsCmdCb.u16BufDataLen  = 0;
  sGpsCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
  sGpsCmdCb.pu8AssembleBuf = OSMemGet(pSmallMem, &err);
  sGpsCmdCb.pu8CmdIndex    = OSMemGet(pSmallMem, &err);
  sGpsCmdCb.u8GpsRxNum     = 0;
  
  //Get One soft timer
  sGpsCmdCb.sGpsDog        = SwdGet();   
  SwdSetName(sGpsCmdCb.sGpsDog, "GpsTimer");
  SwdSetLifeTime(sGpsCmdCb.sGpsDog, 200);
  
#ifndef TEST
        g_bGpsFirstRx = TRUE;
#else
        g_bGpsFirstRx = FALSE; 
#endif
  //GPS接收超时定时器，发送前一分钟打开，定时为50秒，超时后若还没有GPS信号，直接返回上次数据和错误标志。
  g_s8GpsFirstDog = SwdGet();   
  SwdSetName(g_s8GpsFirstDog, "GpsFirstRx");
  SwdSetLifeTime(g_s8GpsFirstDog, 50000);
}

/**********************************************************************************************
 函数名称：GpsCmdRun
 函数功能：GPS 命令解析，连续采集8组有效数据，找出卫星质量好的一组数据

 参数： 无
 返回： 无
**********************************************************************************************/
void GpsCmdRun(void)
{
  UINT8 u8Cmd=GNNON;
  UINT8 GpsStatus=GPS_RX_INVALID;
  
 //获取有效数据包
  while((sGpsCmdCb.eAssembleState != E_UART_CMD_RECEIVING_VALID) && (Uart2GetRxDataLength() > 0))	
  {
    GpsCmdAssemble(Uart2GetByte());

    if(SwdEnable(sGpsCmdCb.sGpsDog) == TRUE)
    {
      SwdDisable(sGpsCmdCb.sGpsDog);
    }
  }
  //数据有效则处理 
  if(sGpsCmdCb.eAssembleState == E_UART_CMD_RECEIVING_VALID)
  {
    u8Cmd = NmeaPackType(sGpsCmdCb.pu8AssembleBuf, sGpsCmdCb.u16BufDataLen);
    if(g_bGpsFirstRx == TRUE)
    {
      if(u8Cmd != GNNON)
      {
        if(g_bDebug & GPS_DEBUG)    //打印调试信息
        {
          Uart3SendBuffer(sGpsCmdCb.pu8AssembleBuf, sGpsCmdCb.u16BufDataLen);
        }
        //建立索引表
        CreatCmdIndex();
        //校准RTC
        if(bRtcFlag == FALSE)
        {
          bRtcFlag = CalibratRtc(u8Cmd);
        }
        //Gps数据处理
        GpsStatus = GpsInfoProcess(u8Cmd);
        if(GpsStatus == GPS_RX_COMPLETE)
        {
          g_bGpsFirstRx = TRUE;
          SwdDisable(g_s8GpsFirstDog);    
          SwdReset(g_s8GpsFirstDog);
    
          //提取计算有效经纬度
          CalibratLocation();
          bRtcFlag = FALSE;
          
          g_bGpsCompleted = TRUE;
         
          //关闭串口2，清空接收缓冲区
          GpsPowerOff();
          Uart2DisableInterrupt();
          Uart2EmptyRxBuf();
        }
      }
    }
    else
    {
#ifndef TEST
        g_bGpsFirstRx = TRUE;
        GpsPowerOff();
#else
        g_bGpsFirstRx = FALSE; 
#endif
        
        //提取计算有效经纬度
        CalibratLocation();
        bRtcFlag = FALSE;
          
        g_bGpsCompleted = TRUE;
         
        //关闭串口2，清空接收缓冲区
        Uart2DisableInterrupt();
        Uart2EmptyRxBuf();
    }
    
    memset(sGpsCmdCb.pu8AssembleBuf, 0, BUF_SMALL_SIZE);
    sGpsCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
    sGpsCmdCb.u16BufDataLen = 0;
  }
  else
  {
    if(SwdEnable(sGpsCmdCb.sGpsDog) == TRUE)
    {
      if(SwdGetLifeTime(sGpsCmdCb.sGpsDog) == 0)
      {
        sGpsCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
        sGpsCmdCb.u16BufDataLen = 0;
        
        SwdReset(sGpsCmdCb.sGpsDog);
        SwdDisable(sGpsCmdCb.sGpsDog);
      }
    }
    else
    {
      if(sGpsCmdCb.u16BufDataLen != 0)
      {
        SwdReset(sGpsCmdCb.sGpsDog);
        SwdEnable(sGpsCmdCb.sGpsDog);
      }
    }
  }
  
  //超时时间到后，关闭超时定时器，将GPS的解析标志关闭，退出解析程序
  if(SwdGetLifeTime(g_s8GpsFirstDog) == 0)
  {
    g_bGpsFirstRx = FALSE;
    
    SwdDisable(g_s8GpsFirstDog);    
    SwdReset(g_s8GpsFirstDog);
  }
}
