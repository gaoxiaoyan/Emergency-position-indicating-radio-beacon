
#ifndef __BDMODULE_H__
#define __BDMODULE_H__

#include "DataType.h"

#ifdef  BD_GLOBAL
#define BD_EXT
#else
#define BD_EXT extern
#endif

//数据包解析协议
typedef enum
{
  //命令帧头
  E_UART_CMD_RECEIVING_HEADER,    
  
  //命令
  E_UART_CMD_RECEIVING_CMD,    
  
  //接收数据总长
  E_UART_CMD_RECEIVING_LENGHT,
  
  //接收数据主体
  E_UART_CMD_RECEIVING_BODY,
  
  //数据有效
  E_UART_CMD_RECEIVING_VALID
		
}teDebugCmdAssembleState;

/* 数据组包结构声明 */
typedef struct
{
  UINT8 *pu8AssembleBuf;  //组包区数据指针
  
  UINT8 u8CmdLen;         //命令长度
  
  UINT16 u16BufDataLen;     //已经组包的数据长度

	UINT8 u8Len;						//接收数据长度为2byte，记录正在接收的高地位
		
  UINT16 u16RevDataLen ;   //每包数据的长度
	
  teDebugCmdAssembleState eAssembleState;  //组包状态
  
  SINT8 sBdDog;
	
}_tsBdCmdCb;

BD_EXT _tsBdCmdCb sBdCmdCb;

////////////////////////////////////////////////////////////////////////////
//                     发送命令结构
//$DWSQ通讯命令结构
typedef struct
{
  UINT8 au8BdCmd[5];          //北斗模块通讯命令
  UINT8 au8DataLen[2];        //数据包长度     
  UINT8 au8BdLocaAddr[3];     //北斗机用户机地址，本机卡号
  
  UINT8 u8InfoType;           //信息类别
  UINT8 au8HighData[4];       //高程数据和天线高
  UINT8 au8Pressure[4];       //气压数据
  UINT8 au8u8Freq[2];         //入站频度
  UINT8 u8Check;              //校验和
}_tsCmdDwsqCb;
BD_EXT _tsCmdDwsqCb sCmdDwsqCb;

//$TXSQ通讯申请命令结构
typedef struct
{
  UINT8 au8BdCmd[5];          //北斗模块通讯命令
  UINT8 au8DataLen[2];        //数据包长度     
  UINT8 au8BdLocaAddr[3];     //北斗机用户机地址，本机卡号
  
  UINT8 u8InfoType;           //信息类别
  UINT8 au8BdSendAddr[3];      //北斗机接收机地址。
  UINT8 au8MsgLen[2];         //报文长度
  UINT8 u8Ack;                //是否应答
  UINT8 *pMsg;                //报文信息指针
  UINT8 u8Check;              //校验和
}_tsCmdTxsqCb;
BD_EXT _tsCmdTxsqCb sCmdTxsqCb;

//$SJSC时间输出命令结构
typedef struct
{
  UINT8 au8BdCmd[5];          //北斗模块通讯命令
  UINT8 au8DataLen[2];        //数据包长度     
  UINT8 au8BdLocaAddr[3];     //北斗机用户机地址，本机卡号
  
  UINT8 au8Freq[2];           //输出频度
  UINT8 u8Check;              //校验和
}_tsCmdSjscCb;
BD_EXT _tsCmdSjscCb sCmdSjscCb;

////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//                    接收命令结构声明
//$GLJC功率检测命令结构
typedef struct
{
  UINT8 au8BdCmd[5];          //北斗模块通讯命令
  UINT8 au8DataLen[2];        //数据包长度     
  UINT8 au8BdLocaAddr[3];     //北斗机用户机地址，本机卡号
  
  UINT8 u8Freq;               //输出频度
}_tsCmdGljcCb;
BD_EXT _tsCmdGljcCb sCmdGljcCb;

typedef struct
{
  UINT8 au8BD_X[10];
  UINT8 au8BD_Y[9];
}_tsBdLocation;

//$DWXX定位信息命令结构
typedef struct
{
  UINT8 au8BdCmd[5];          //北斗模块通讯命令
  UINT8 au8DataLen[2];        //数据包长度     
  UINT8 au8BdLocaAddr[3];     //北斗机用户机地址，本机卡号
  
  UINT8 u8InfoType;           //信息类别
  UINT8 au8BdFindAddr[3];     //北斗机接收机地址。
  
  //位置数据
  UINT8 au8TimeData[4];
  UINT8 au8BD_X[4];        //经度信息      
  UINT8 au8BD_Y[4];        //纬度信息
  UINT8 au8HData[2];
  UINT8 au8uHData[2];
  
  UINT8 u8Check;              //校验和
}_tsCmdDwxxCb;
BD_EXT _tsCmdDwxxCb sCmdDwxxCb;

//$TXXX通信信息命令结构
typedef struct
{
  UINT8 au8BdCmd[5];          //北斗模块通讯命令
  UINT8 au8DataLen[2];        //数据包长度     
  UINT8 au8BdLocaAddr[3];     //北斗机用户机地址，本机卡号
  
  UINT8 u8InfoType;           //信息类别
  UINT8 au8SendAddr[3];       //发送方地址
  UINT8 u8SendHour;           //发送时间，小时
  UINT8 u8SendMinute;         //发送时间，分钟。
  
  UINT8 au8MsgLen[2];         //电文长度
  UINT8 *pMsg;                //电文内容地址
  UINT8 u8Crc;                //CRC标志
  
  UINT8 u8Check;              //校验和
}_tsCmdTxxxCb;
BD_EXT _tsCmdTxxxCb sCmdTxxxCb;

//$SJXX时间信息命令结构
typedef struct
{
  UINT8 au8BdCmd[5];          //北斗模块通讯命令
  UINT8 au8DataLen[2];        //数据包长度     
  UINT8 au8BdLocaAddr[3];     //北斗机用户机地址，本机卡号
  
  UINT8 au8Year[2];           //年
  UINT8 u8Month;              //月
  UINT8 u8Date;               //日
  UINT8 u8Hour;               //时
  UINT8 u8Minuth;             //分
  UINT8 u8Second;             //秒
  
  UINT8 u8Check;              //校验和
}_tsCmdSjxxCb;
BD_EXT _tsCmdSjxxCb sCmdSjxxCb;

//$FKXX反馈信息命令结构
typedef struct
{
  UINT8 au8BdCmd[5];          //北斗模块通讯命令
  UINT8 au8DataLen[2];        //数据包长度     
  UINT8 au8BdLocaAddr[3];     //北斗机用户机地址，本机卡号
  
  UINT8 u8BackInfo;           //反馈标志
  UINT8 u8OtherInfo[4];       //其他信息
  
  UINT8 u8Check;              //校验和
}_tsCmdFkxxCb;
BD_EXT _tsCmdFkxxCb sCmdFkxxCb;


/****************************************************************************
函数名称：BdModuleInit()
功    能：北斗模块初始化

参    数：无
返回值  ：无
*****************************************************************************/
extern void BdModuleInit(void);

/***************************************************************************
//函数名称：BdCmdRun
//功能描述：串口接收命令解析
//
//参数：无
//返回：无
***************************************************************************/
extern void BdCmdRun(void);

/****************************************************************************
函数名称：BdSendCmdTXSQ()
功    能：北斗模块通讯申请

参    数：无
返回值  ：无
*****************************************************************************/
extern void BdSendCmdTXSQ(UINT8 *pSendAddr, UINT8 *pMsg, UINT8 u8MsgLen);
#endif