

/*
  GPS 指令解析头文件
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


//GPS 所需信息结构声明
typedef struct
{
  //定位有效标志
  BOOL bActive;

  // UTC时间
  UINT8 UTC_Time[6];
  UINT8 UTC_Date[6];

  // 纬度
  UINT8 GPS_X[10];
  // 纬度半球
  UINT8 NorthSouth;

  // 经度
  UINT8 GPS_Y[11];
  //经度半球
  UINT8 EastWest;

  //地面速率（000.0 ~999.9）   ******************************************/
  UINT8 Speed[8];

  //地面航向（000.0~359.9， 以真北方向为基准）
  UINT8 Course[8];

  //使用卫星数量
  UINT8 UseEphSum;

}_tsGPSInfo;


#define GPS_MAX_SIZE      8
//外部结构声明，主要用来接收缓冲器的数据
GPS_EXT _tsGPSInfo sGPSBuf[GPS_MAX_SIZE];


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
  
  UINT8 *pu8RxDataBuf;    //备份区数据指针
  
  BOOL bGpsParse;         //数据解析标志，TRUE--正在解析，FALSE -- 解析完毕 
		
  UINT16 u16BufDataLen ;  //组包的数据长度
  
  UINT16 u16RxDataLen;    //接收备份区数据长度
	
  teUartCmdAssembleState eAssembleState;  //组包状态
  
  SINT8 sGpsDog;
	
}_tsUartCmdCb;

GPS_EXT _tsUartCmdCb sGpsCmdCb;


/***********************************************************************************
函数名称：Uart0CmdInit()
函数功能：GPS数据组包结构初始化

参数：    无
返回：    无
***********************************************************************************/
extern void GpsCmdInit(void);

/**********************************************************************************************
 函数名称：GpsCmdRun
 函数功能：GPS 命令解析，连续采集8组有效数据，找出卫星质量好的一组数据

 参数： 无
 返回： 无
**********************************************************************************************/
extern void GpsCmdRun(void);




#endif



