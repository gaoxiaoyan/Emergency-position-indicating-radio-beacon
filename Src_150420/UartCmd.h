
/******************************************************************************
*											
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved										
*
* 文 件 名： UartCmd.h
* 描       述：

* 创建日期： 2014年12月5日11:31:59
* 作    者： Bob
* 当前版本： V1.00

------------------------------------------------------
*******************************************************************************/
#ifndef __UART_CMD_H__
#define __UART_CMD_H__

//定义组包类型，4路UART的接收类型
#define TYPE_ERROR              0u
#define GPS_TYPE                1u
#define DTU_TYPE                2u
#define USB_TYPE                3u
#define WIRELESS                4u

//配置命令校验
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

//返回的配置状态
#define DTU_CFG_OK                  1u
#define CFG_CHECK_ERROR             2u
#define CFG_ERROR                   3u

//
#define PZNON             0u    //无配置
#define XXPZ              1u    //信息配置
#define DMPZ              2u    //主服务器配置
#define S1PZ              3u    //中心1配置
#define S2PZ              4u    //中心2配置
#define S3PZ              5u    //中心3配置
#define S4PZ              6u    //中心3配置
#define ZDQD              7u    //设备启动
#define ZDTZ              8u    //设备停止
#define ZDGJ              9u    //设备关机
#define TSKG              10u   //调试开关
#define BBHM              11u   //版本号码
#define DTHM              12u   //读取dtu配置信息
#define YJCX              13u


//数据帧指令编号
#define XXPZ_DATA_LEN               1u
#define XXPZ_TML_INDEX              2u
//#define XXPZ_TML_TYPE               3u
#define XXPZ_TML_INFO               3u
#define XXPZ_SEND_TIME              4u
#define XXPZ_RESERVED               5u
#define XXPZ_CHECK_SUM              6u

//配置使能状态
#define DTU_CFG_DISABLE             0u
#define DTU_CFG_ENABLE              1u
#define DTU_CFG_VIEW                2u

//主服务器配置命令
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

//分中心1配置
#define S1PZ_CFG_LEN                1u
#define S1PZ_CFG_EN                 2u
#define S1PZ_CFG_IP                 3u
#define S1PZ_CFG_COM                4u
#define S1PZ_CFG_RESERVED           5u
#define S1PZ_CFG_CHK                6u

//分中心2配置
#define S2PZ_CFG_LEN                1u
#define S2PZ_CFG_EN                 2u
#define S2PZ_CFG_IP                 3u
#define S2PZ_CFG_COM                4u
#define S2PZ_CFG_RESERVED           5u
#define S2PZ_CFG_CHK                6u

//分中心3配置
#define S3PZ_CFG_LEN                1u
#define S3PZ_CFG_EN                 2u
#define S3PZ_CFG_IP                 3u
#define S3PZ_CFG_COM                4u
#define S3PZ_CFG_RESERVED           5u
#define S3PZ_CFG_CHK                6u

//分中心4配置
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
//  BOOL  bCfgEnable;               //配置使能
  BOOL  bCfgTcpUdp;               //TCP/UDP协议切换，0--TCP/1--UDP
  BOOL  bCfgApn;                  //APN码，0--中国移动，1--中国联通
  UINT8 u8CfgUserNameLen;        //用户名长度
  UINT8 au8CfgUserName[16];      //用户名称
  UINT8 u8CfgPasswordLen;        //用户密码长度
  UINT8 au8CfgPassword[16];      //用户密码
  UINT8 u8CfgIpLen;              //ip地址长度
  UINT8 au8CfgIpAddr[64];        //IP地址
  UINT8 au8CfgComIndex[2];       //端口号
  UINT8 au8CfgId[11];            // -- 到此位置为105个byte
  
  BOOL  bBckTcpUdp;               //备份TCP/UDP协议切换，0--TCP/1--UDP
  BOOL  bBckApn;                  //备份APN码，0--中国移动，1--中国联通
  UINT8 u8BckUserNameLen;         //备份用户名长度
  UINT8 au8BckUserName[16];      //备份用户名称
  UINT8 u8BckPasswordLen;        //备份密码长度
  UINT8 au8BckPassword[16];      //备份用户密码
  UINT8 u8BckIpLen;              //ip地址长度
  UINT8 au8BckIpAddr[64];        //备份IP地址
  UINT8 au8BckComIndex[2];        //备份端口号 
  UINT8 au8BckId[11];             // -- 从备份区开始到此位置为105个byte
  
}_tsMainServer;

extern _tsMainServer sMainServer;
#endif

//DTU配置
typedef struct
{
//  BOOL    bCfgEnable;          //IP配置使能
  
  UINT8   au8CfgIpAddr[60];    //IP地址
  UINT8   u8CfgIpLen;          //IP地址长度
  BOOL    bCfgEnable;          //配置使能
  UINT8   au8CfgComIndex[2];   //端口号
  
  UINT8   au8BckIpAddr[60];    //备份IP地址
  UINT8   u8BckIpLen;          //备份IP地址长度
  BOOL    bBckEnable;          //配置使能
  UINT8   au8BckComIndex[2];   //备份端口号
}_tsDtuCfg;

//定义5组DTU配置
extern _tsDtuCfg sDtuCfg[4];

//接收缓冲区最大长度
#define MAX_BODY_SIZE		100u

//定义GPS接收缓冲区
extern UINT8 au8GpsBodyBuf[MAX_BODY_SIZE];
//定义DTU接收缓冲区
extern UINT8 au8DtuBodyBuf[MAX_BODY_SIZE];
//定义USB接收缓冲区
extern UINT8 au8UsbBodyBuf[MAX_BODY_SIZE];

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
  UINT8 *pu8AssembleBuf;
		
  UINT8 u8BufDataLength;
	
  teUartCmdAssembleState eAssembleState;
	
}_tsUartCmdCb;

extern _tsUartCmdCb sUartCmdCb[4];

/***************************************************************************
//函数名称：UartCmdInit
//功能描述：串口接收命令解析
//
//参数：无
//返回：无
***************************************************************************/
extern void UartCmdInit(void);

/***************************************************************************
//函数名称：UartCmdRun
//功能描述：串口接收命令解析
//
//参数：无
//返回：无
***************************************************************************/
extern void UartCmdRun(void);

/***********************************************************************************
  函数名称：CreatCmdIndex
  函数功能：寻找逗号位置，创建命令索引，讲原有的逗号替换为0x00

  输入：pu8Buf -- 接收GPS数据的缓冲区
  输出：无
***********************************************************************************/
extern void CreatCmdIndex(uint8 * pu8Buf);

/***********************************************************************************
  函数名称：RealCmdProcess
  函数功能：查找GPS数据第N个参数的偏移

  输入：创建索引后的接收GPS数据缓冲区
  输出：返回第N个","之后的信息，需要*buffer有效并创建索引后才可以执行
***********************************************************************************/
extern uint8* RealCmdProcess( uint8* pu8Buf, uint8 num );

/**********************************************************************************************
 函数名称：ConfigInfoProcess
 函数功能：配置命令处理函数，将配置信息copy到信息数据结构，打开保存。

 参数： u8Index   需要解析的通道，USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 返回： 校验和状态
**********************************************************************************************/
extern UINT8 ConfigInfoProcess(UINT8 u8Index, UINT8 *pu8RxBuffer);

/**********************************************************************************************
 函数名称：ConfigPackType
 函数功能：配置命令解析

 参数： 无
 返回： 命令类型
**********************************************************************************************/
extern SINT8 ConfigPackType(const UINT8 *pu8Buf, UINT8 u8Size);

/**********************************************************************************************
 函数名称：MainServerConfigProcess
 函数功能：DTU主服务器配置命令。

 参数： u8Index   需要解析的通道，USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 返回： 校验和状态
**********************************************************************************************/
extern UINT8 MainServerConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer);

/**********************************************************************************************
 函数名称：Sub1ConfigProcess
 函数功能：DTU分中心1配置命令。

 参数： u8Index   需要解析的通道，USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 返回： 校验和状态
**********************************************************************************************/
extern UINT8 Sub1ConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer);

/**********************************************************************************************
 函数名称：Sub2ConfigProcess
 函数功能：DTU分中心1配置命令。

 参数： u8Index   需要解析的通道，USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 返回： 校验和状态
**********************************************************************************************/
extern UINT8 Sub2ConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer);

/**********************************************************************************************
 函数名称：Sub3ConfigProcess
 函数功能：DTU分中心1配置命令。

 参数： u8Index   需要解析的通道，USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 返回： 校验和状态
**********************************************************************************************/
extern UINT8 Sub3ConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer);

/**********************************************************************************************
 函数名称：Sub4ConfigProcess
 函数功能：DTU分中心1配置命令。

 参数： u8Index   需要解析的通道，USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 返回： 校验和状态
**********************************************************************************************/
extern UINT8 Sub4ConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer);

/**********************************************************************************************
 函数名称： DebugProcess
 函数功能：debug接口开启关闭处理。

 参数： u8Index   需要解析的通道，USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 返回： 校验和状态
**********************************************************************************************/
extern UINT8 DebugProcess(UINT8 u8Index, UINT8 *pu8RxBuffer);


/**********************************************************************************************
 函数名称： GetVersionProcess
 函数功能：获取版本信息

 参数： u8Index   需要解析的通道，USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 返回： 校验和状态
**********************************************************************************************/
extern UINT8 GetVersionProcess(UINT8 u8Index, UINT8 *pu8RxBuffer);


/**********************************************************************************************
 函数名称： PowerDownProcess
 函数功能：终端关机指令

 参数： u8Index   需要解析的通道，USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 返回： 校验和状态
**********************************************************************************************/
extern UINT8 PowerDownProcess(UINT8 u8Index, UINT8 *pu8RxBuffer);

/**********************************************************************************************
 函数名称： ReadDtuConfigProcess
 函数功能：获取DTU的标示符

 参数： u8Index   需要解析的通道，USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 返回： 校验和状态
**********************************************************************************************/
extern UINT8 ReadDtuConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer);

#endif