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
#ifdef MSP430F149
#include "msp430x14x.h"
#else
#include "msp430x54x.h"
#endif

#include "global.h"
#include "DataType.h"
#include "GpsParse.h"
#include "DtuModule.h"
#include "UartCmd.h"
#include "Timer.h"
#include "Display.h"
#include "SystemCtl.h"

_tsUartCmdCb sUsbCmdCb;

//DTU接收缓冲区
UINT8 au8UsbBodyBuf[MAX_BODY_SIZE];

//前面的强制类型转化缺省会造成计算错误
#define _GetWordFromAddr(pAddr)         (UINT16)(((UINT16)pAddr[0]) << 8) + pAddr[1]

#define _GetDoubleWordFromAddr(pAddr)   ((UINT32)(((UINT32)pAddr[0]) << 24) +    \
                                         (UINT32)(((UINT32)pAddr[1]) << 16) +    \
                                         (UINT32)(((UINT32)pAddr[2]) << 8) + pAddr[3])


const UINT8 *pCfgCmd[] = {"$XXPZ", "$DMPZ", "$S1PZ", "$S2PZ", "$S3PZ", "$S4PZ", "$ZDQD", "$ZDTZ", "$ZDGJ", "$TSKG", "$BBHM", "$YJCX"};


/* ====================================================================================================== */
//                                  配置命令共用函数定义
/* ====================================================================================================== */

/***********************************************************************************
  函数名称：CreatCmdIndex
  函数功能：寻找逗号位置，创建命令索引，讲原有的逗号替换为0x00

  输入：pu8Buf -- 接收命令数据的缓冲区
  输出：无
***********************************************************************************/
UINT8 au8CmdIndex[32];    //逗号索引保存表
void CreatCmdIndex(UINT8 * pu8Buf)
{
  UINT8 i, len;
  UINT8 j=0;

  MEM_SET(au8CmdIndex, 0, sizeof(au8CmdIndex));

  len = MAX_BODY_SIZE;
  for(i=0; i<len; i++)
  {
     if(pu8Buf[i] == ',')
     {
        au8CmdIndex[j++] = i;
//        pu8Buf[i] = 0x00;
     }
  }
}

/***********************************************************************************
  函数名称：RealCmdProcess
  函数功能：查找命令数据第N个参数的偏移

  输入：创建索引后的接收GPS数据缓冲区
  输出：返回第N个","之后的信息，需要*buffer有效并创建索引后才可以执行
***********************************************************************************/
UINT8* RealCmdProcess( UINT8* pu8Buf, UINT8 num )
{
  if ( num < 1 )
    return  &pu8Buf[0];
  return  &pu8Buf[ au8CmdIndex[num - 1] + 1];
}

/**********************************************************************************************
 函数名称：CalcCheckSum
 函数功能：计算buf的校验和，遇到*返回

 参数： *pu8Buf -- buf指针
         u8Len  -- 数据长度
 返回： 校验和状态
**********************************************************************************************/
UINT8 CalcCheckSum(UINT8 *pu8Buf, UINT8 u8Len)
{
  UINT8 u8CheckSum = 0;
  UINT8 i;
  
  //计算发送校验和
  for(i=1; i<u8Len; i++)
  {
    if(pu8Buf[i] != '*')
    {
      u8CheckSum ^= pu8Buf[i];
    }
    else
    {
      break;
    }
  }
  
  return u8CheckSum;
}

/**********************************************************************************************
 函数名称：CalcDataLen
 函数功能：计算buf中接收数据帧的数据长度

 参数： *pu8Buf -- buf指针
        u8Len  -- 数据长度
 返回： 数据包的长度
**********************************************************************************************/
UINT16 CalcDataLen(UINT8 *pu8Buf, UINT16 u16Len)
{
  UINT16 u16DataLen = 0;
  UINT16 i;
  
  //计算发送校验和
  for(i=0; i<u16Len; i++)
  {
    if(pu8Buf[i] == '*')
    {
      u16DataLen = i + 2;
      break;
    }
  }
  
  return u16DataLen;
}


/**********************************************************************************************
 函数名称：ConfigInfoProcess
 函数功能：配置命令处理函数，将配置信息copy到信息数据结构，打开保存。

 参数： u8Index   需要解析的通道，USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 返回： 校验和状态
**********************************************************************************************/
UINT8 ConfigInfoProcess(UINT8 u8Index, UINT8 *pu8RxBuffer)
{
  UINT8 i=0;
  UINT8 *pu8Buf;
  UINT8 *pu8Temp;
  UINT16 u16DataLen=0;
  UINT8 u8CheckSum=0;
  UINT8 au8SendBuf[MAX_BODY_SIZE];
  UINT8 u8Temp=0;   //存放配置状态的在数组中的下标位置
  
  if((u8Index != USB_TYPE) && (u8Index != DTU_TYPE))
  {
    return TYPE_ERROR;
  }
  
  //计算校验和
  for(i=1; i<MAX_BODY_SIZE; i++)
  {
    if(pu8RxBuffer[i] != '*')
    {
      u8CheckSum ^= pu8RxBuffer[i];
    }
    else
    {
      //将数据长度更新为实际的数据长度
      u16DataLen = i+2;
      break;
    }
  }
  
  MEM_SET(au8SendBuf, 0, MAX_BODY_SIZE);
  MEM_CPY(au8SendBuf, pu8RxBuffer, u16DataLen+1);

  au8SendBuf[1] = 'P';
  au8SendBuf[2] = 'Z';
  au8SendBuf[3] = 'F';
  au8SendBuf[4] = 'K';
  //建立索引表
//  CreatCmdIndex(pu8RxBuffer);          
  
  //计算实际数据长度存入buffer
  pu8Buf = RealCmdProcess(au8SendBuf, XXPZ_DATA_LEN);
  pu8Buf++;
  u8Temp = pu8Buf - au8SendBuf;
  au8SendBuf[u8Temp] = u16DataLen;
  
  //获取校验和
  pu8Temp = RealCmdProcess(au8SendBuf, XXPZ_CHECK_SUM);
  pu8Temp++;
  if(u8CheckSum != *pu8Temp)
  {  
    //发送状态赋值
    pu8Buf = RealCmdProcess(au8SendBuf, XXPZ_RESERVED);
    u8Temp = pu8Buf - au8SendBuf;
    au8SendBuf[u8Temp] = CFG_ERROR;
    
    u8CheckSum = 0;
    //计算发送校验和
    for(i=1; i<u16DataLen; i++)
    {
      if(au8SendBuf[i] != '*')
      {
        u8CheckSum ^= au8SendBuf[i];
      }
      else
      {
        break;
      }
    }
    u8Temp = pu8Temp - au8SendBuf;
    au8SendBuf[u8Temp] = u8CheckSum;
    
    if(u8Index == USB_TYPE)
    {
      UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
      }
    }
    
    return CMD_XXPZ_ERROR;
  }
  else
  {
    
    //发送状态赋值
    pu8Buf = RealCmdProcess(au8SendBuf, XXPZ_RESERVED);
    u8Temp = pu8Buf - au8SendBuf;
    au8SendBuf[u8Temp] = DTU_CFG_OK;
      
    u8CheckSum = 0;
    //计算发送校验和
    for(i=1; i<u16DataLen; i++)
    {
      if(au8SendBuf[i] != '*')
      {
        u8CheckSum ^= au8SendBuf[i];
      }
      else
      {
        break;
      }
    }
    u8Temp = pu8Temp - au8SendBuf;
    au8SendBuf[u8Temp] = u8CheckSum;
  
    if(u8Index == USB_TYPE)
    {
      UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
      }
    }
  ////////////////////////////////////////////////////////////////////////////////////////////////// 
    //                           浮点运算，比较耗时
    //获取设备编号
    pu8Buf = RealCmdProcess(pu8RxBuffer, XXPZ_TML_INDEX);
    sDisplay.u32TmlIndex = _GetDoubleWordFromAddr(pu8Buf);
    sDisplay.u32TmlIndex = sDisplay.u32TmlIndex % 1000000;
    //设备编号换算为6位显示编码
    sDisplay.au8TmlIndex[0] = (UINT8)(sDisplay.u32TmlIndex/100000 + 0x30);
    sDisplay.au8TmlIndex[1] = (UINT8)(sDisplay.u32TmlIndex/10000%10 + 0x30);
    sDisplay.au8TmlIndex[2] = (UINT8)(sDisplay.u32TmlIndex/1000%10 + 0x30);
    sDisplay.au8TmlIndex[3] = (UINT8)(sDisplay.u32TmlIndex/100%10 + 0x30);
    sDisplay.au8TmlIndex[4] = (UINT8)(sDisplay.u32TmlIndex/10%10 + 0x30);
    sDisplay.au8TmlIndex[5] = (UINT8)(sDisplay.u32TmlIndex%10 + 0x30);
  //////////////////////////////////////////////////////////////////////////////////////////////////   
    MEM_SET(&sDisplay.au8TmlInfo[0], ' ', 32);
/*    
    //去掉该段代码，直接由PC发送汉字编码
    //获取设备类型
    sDisplay.u8TmlType = *RealCmdProcess(pu8RxBuffer, XXPZ_TML_TYPE);
    if(sDisplay.u8TmlType == E_TML_TYPE_MACHINE) 
    {
      MEM_CPY(&sDisplay.au8TmlInfo[0], "收油机", 6);
    }
    else 
    {
      if(sDisplay.u8TmlType == E_TML_TYPE_RAIL) 
      {
        MEM_CPY(&sDisplay.au8TmlInfo[0], "围油栏", 6);
      }
    }
*/   
    //获取设备型号
    pu8Buf = RealCmdProcess(pu8RxBuffer, XXPZ_TML_INFO);
    pu8Temp = RealCmdProcess(pu8RxBuffer, XXPZ_SEND_TIME);
    //获取设备型号信息的长度
    sDisplay.u8InfoLen = pu8Temp - pu8Buf - 1;  //两个逗号之间是设备型号的长度，还要减去1个逗号。
    for(i=0; i<sDisplay.u8InfoLen; i++)
    {
       sDisplay.au8TmlInfo[i] = *pu8Buf;
       pu8Buf++;
    }
    
    //获取时间间隔
    pu8Buf = RealCmdProcess(pu8RxBuffer, XXPZ_SEND_TIME);
    if((*pu8Buf > 0) && (*pu8Buf < 7))
    {
      sDisplay.u8TimeSpace = *pu8Buf;
    }
    else
    {
      if(*pu8Buf < 1)
      {
        sDisplay.u8TimeSpace = E_TIME_MINUTE_5;
      }
      else if(*pu8Buf > 6)
      {
        sDisplay.u8TimeSpace = E_TIME_MINUTE_30;
      }
    }
    sDisplay.u8TimeWake = sDisplay.u8TimeSpace * 5;//转化为实际的分钟数保存
//    sDisplay.u32TimeSpace = Index2Second(sDisplay.u8TimeSpace);
    
    //打开保存使能
    g_u8NeedSaved |= SAVE_TML_INFO;
    //打开显示刷新
    sDisplay.bRefresh = TRUE;
    
    return CMD_XXPZ_OK;
  }
}

//此函数为获取字符串长度
UINT8 CalcStringLen(UINT8 *s)
{
  UINT8 i=0;
  
  while(s[i] != 0)
  {
    i++;
  }
  
  if(i>0)
    return i;
  else
    return 0;
}

/**********************************************************************************************
 函数名称：MainServerConfigProcess
 函数功能：DTU主服务器配置命令。

 参数： u8Index   需要解析的通道，USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 返回： 校验和状态
**********************************************************************************************/
UINT8 MainServerConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer)
{
  UINT8 i;
  UINT8 *pu8Buf, *pu8Temp;
  UINT8 u8Temp;
  UINT16 u16DataLen=0;
  UINT8 u8CheckSum=0;
  UINT8 au8SendBuf[MAX_BODY_SIZE];
  UINT8 u8State = 0xff;
  BOOL bSaved = FALSE;
  
  if((u8Index != USB_TYPE) && (u8Index != DTU_TYPE))
  {
    return TYPE_ERROR;
  }
  
  //计算整个数据包的校验和，数据长度
  u8CheckSum = CalcCheckSum(pu8RxBuffer, MAX_BODY_SIZE);
  u16DataLen = CalcDataLen(pu8RxBuffer, MAX_BODY_SIZE);
    
  MEM_SET(au8SendBuf, 0, MAX_BODY_SIZE);
  MEM_CPY(au8SendBuf, pu8RxBuffer, u16DataLen + 1);
  
  //发送BUf命令初始化
  au8SendBuf[1] = 'D';
  au8SendBuf[2] = 'M';
  au8SendBuf[3] = 'F';
  au8SendBuf[4] = 'K';
  
  pu8Temp = RealCmdProcess(au8SendBuf, DMPZ_CFG_CHK);
  pu8Temp++;
  //比对校验和是否正确
  if(u8CheckSum != *pu8Temp)
  {
    //发送状态赋值
    pu8Buf = RealCmdProcess(au8SendBuf, DMPZ_CFG_RESERVED);
    u8Temp = pu8Buf - au8SendBuf;
    au8SendBuf[u8Temp] = DTU_CFG_ERROR;
    
    //计算发送校验和
    u8CheckSum = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
    u8Temp = pu8Temp - au8SendBuf;
    au8SendBuf[u8Temp] = u8CheckSum;
    
    if(u8Index == USB_TYPE)
    {
      UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
      }
    }
    
    return CMD_DMPZ_ERROR;
  }
  else
  {
    //提取配置使能
    pu8Buf = RealCmdProcess(au8SendBuf, DMPZ_CFG_EN);
    if(*pu8Buf == DTU_CFG_VIEW)    //DTU不需要配置，中心查看DTU的信息
    {
      //获取备份协议
      pu8Buf = RealCmdProcess(au8SendBuf, DMPZ_CFG_TU);
      u8Temp = pu8Buf - au8SendBuf;
      au8SendBuf[u8Temp++] = sMainServer.bBckTcpUdp;
      au8SendBuf[u8Temp++] = ',';
      //获取备份APN码
      au8SendBuf[u8Temp++] = sMainServer.bBckApn;
      au8SendBuf[u8Temp++] = ',';
      //获取备份用户名
      i = CalcStringLen(&sMainServer.au8BckUserName[0]);
      if(i>16) i = 16;
      MEM_CPY(&au8SendBuf[u8Temp], &sMainServer.au8BckUserName[0], i);
      u8Temp += i;
      au8SendBuf[u8Temp++] = ',';
      //获取备份密码
      i = CalcStringLen(&sMainServer.au8BckUserName[0]);
      if(i>16) i = 16;
      MEM_CPY(&au8SendBuf[u8Temp], &sMainServer.au8BckPassword[0], i);
      u8Temp += i;
      au8SendBuf[u8Temp++] = ',';
      //获取IP地址
      i = CalcStringLen(&sMainServer.au8BckIpAddr[0]);
      if(i>64) i = 64;
      MEM_CPY(&au8SendBuf[u8Temp], &sMainServer.au8BckIpAddr[0], i);
      u8Temp += i;
      au8SendBuf[u8Temp++] = ',';
      //获取端口号
      au8SendBuf[u8Temp++] = sMainServer.au8BckComIndex[0];
      au8SendBuf[u8Temp++] = sMainServer.au8BckComIndex[1];
      au8SendBuf[u8Temp++] = ',';
      //发送状态
      au8SendBuf[u8Temp++] = DTU_CFG_OK;
      au8SendBuf[u8Temp++] = ',';
      
      au8SendBuf[u8Temp++] = '*';
      au8SendBuf[u8Temp++] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
      au8SendBuf[u8Temp]   = '\r';
      
      //获取数据长度位置
      pu8Buf = RealCmdProcess(au8SendBuf, DMPZ_CFG_LEN);
      pu8Buf++;
      i = pu8Buf - au8SendBuf;
      au8SendBuf[i]   = u8Temp;
      u16DataLen = u8Temp;
    }
    else
    {
      if(*pu8Buf == DTU_CFG_ENABLE)
      {
        pu8Buf = RealCmdProcess(au8SendBuf, DMPZ_CFG_RESERVED);
        u8Temp = pu8Buf - au8SendBuf;
        au8SendBuf[u8Temp] = DTU_CFG_OK;
        
        u8Temp = pu8Temp - au8SendBuf;
        au8SendBuf[u8Temp] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
        
        //获取数据长度位置
        pu8Buf = RealCmdProcess(au8SendBuf, DMPZ_CFG_LEN);
        pu8Buf++;
        i = pu8Buf - au8SendBuf;
        au8SendBuf[i]  = CalcDataLen(au8SendBuf, MAX_BODY_SIZE);
        u16DataLen = au8SendBuf[i];
        
        bSaved = TRUE;
    
      }
    }
    //发送反馈信息
    if(u8Index == USB_TYPE)
    {
       UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
      }
    }
    g_u8TimerDelay = 0;
    while(g_u8TimerDelay < 10);
    
    //配置DTU主服务器设置
    if(bSaved == TRUE)
    {
      //清空主服务器配置缓冲区
      MEM_SET(&sMainServer.bCfgTcpUdp, 0, (sizeof(_tsMainServer)/2));
//=================================================================================================================================
//    更改      
      //获取物资ID作为DTU的ID
      pu8Buf = RealCmdProcess(pu8RxBuffer, DMPZ_CFG_ID);
      pu8Temp = RealCmdProcess(pu8RxBuffer, DMPZ_CFG_EN);
      u8Temp = pu8Temp - pu8Buf - 1;  //获取DTU标示符长度
      MEM_CPY(&sMainServer.au8CfgId[0], pu8Buf, u8Temp);
      
//=================================================================================================================================
     //获取TCP、UDP配置
      pu8Buf = RealCmdProcess(pu8RxBuffer, DMPZ_CFG_TU);
      sMainServer.bCfgTcpUdp = *pu8Buf;

      //获取APN码
      pu8Buf = RealCmdProcess(pu8RxBuffer, DMPZ_CFG_APN);
      sMainServer.bCfgApn = *pu8Buf;

      //获取配置用户名
      pu8Buf = RealCmdProcess(pu8RxBuffer, DMPZ_CFG_NAME);
      pu8Temp = RealCmdProcess(pu8RxBuffer, DMPZ_CFG_PWD);
      sMainServer.u8CfgUserNameLen = pu8Temp - pu8Buf - 1;  //减去一个分隔符空间
      MEM_CPY(&sMainServer.au8CfgUserName[0], pu8Buf, sMainServer.u8CfgUserNameLen);
      
      //获取配置用户密码
      pu8Buf = RealCmdProcess(pu8RxBuffer, DMPZ_CFG_PWD);
      pu8Temp = RealCmdProcess(pu8RxBuffer, DMPZ_CFG_IP);
      sMainServer.u8CfgPasswordLen = pu8Temp - pu8Buf -1;  //减去一个分隔符空间
      MEM_CPY(&sMainServer.au8CfgPassword[0], pu8Buf, sMainServer.u8CfgPasswordLen);
      
      //获得IP地址长度
      pu8Buf = RealCmdProcess(pu8RxBuffer, DMPZ_CFG_IP);
      pu8Temp = RealCmdProcess(pu8RxBuffer, DMPZ_CFG_COM);
      sMainServer.u8CfgIpLen = pu8Temp - pu8Buf - 1;     //减去一个分隔符空间
      MEM_CPY(&sMainServer.au8CfgIpAddr[0], pu8Buf, sMainServer.u8CfgIpLen);
      
      //获得配置端口
      sMainServer.au8CfgComIndex[0] = *pu8Temp++;
      sMainServer.au8CfgComIndex[1] = *pu8Temp;
      
      if(0 != MEM_CMP(&sMainServer.bCfgTcpUdp, &sMainServer.bBckTcpUdp, (sizeof(_tsMainServer)/2)))
      { 
        MEM_SET(&sMainServer.bBckTcpUdp, 0, (sizeof(_tsMainServer)/2));
        MEM_CPY(&sMainServer.bBckTcpUdp, &sMainServer.bCfgTcpUdp, (sizeof(_tsMainServer)/2));
        
        //关闭串口0和2
        UartDisableInterrupt(UART0);
        UartDisableInterrupt(UART2);
        
        u8State = McuConfigDtuMainServer();
        
        UartEnableInterrupt(UART0);
        UartEnableInterrupt(UART2);
        
        if(u8State == DTU_CFG_OK)
          return CMD_DMPZ_OK;
       else
       {
          pu8Buf = RealCmdProcess(au8SendBuf, DMPZ_CFG_RESERVED);
          u8Temp = pu8Buf - au8SendBuf;
          au8SendBuf[u8Temp] = u8State;   //配置失败，把具体的错误状态码再次发送给中心。
          
          u8Temp = pu8Temp - au8SendBuf;
          au8SendBuf[u8Temp] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
          
          //获取数据长度位置
          pu8Buf = RealCmdProcess(au8SendBuf, DMPZ_CFG_LEN);
          pu8Buf++;
          i = pu8Buf - au8SendBuf;
          au8SendBuf[i]   = CalcStringLen(au8SendBuf);
          
          //发送反馈信息
          if(u8Index == USB_TYPE)
          {
             UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
          }
          else
          {
            if(u8Index == DTU_TYPE)
            {
              UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
            }
          }
          
          return CMD_DMPZ_ERROR;
       }
      }
    }    
  } 
  return CMD_DMPZ_OK;
}

/**********************************************************************************************
 函数名称：Sub1ConfigProcess
 函数功能：DTU分中心1配置命令。

 参数： u8Index   需要解析的通道，USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 返回： 校验和状态
**********************************************************************************************/
UINT8 Sub1ConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer)
{
  UINT8 i;
  UINT8 *pu8Buf, *pu8Temp;
  UINT8 u8Temp;
  UINT16 u16DataLen=0;
  UINT8 u8CheckSum=0;
  UINT8 au8SendBuf[MAX_BODY_SIZE];
  UINT8 u8State = 0xff;
  BOOL bSaved = FALSE;
  UINT8 u8CfgEnable;
  
  if((u8Index != USB_TYPE) && (u8Index != DTU_TYPE))
  {
    return TYPE_ERROR;
  }
  
  //计算整个数据包的校验和，数据长度
  u8CheckSum = CalcCheckSum(pu8RxBuffer, MAX_BODY_SIZE);
  u16DataLen = CalcDataLen(pu8RxBuffer, MAX_BODY_SIZE);
    
  MEM_SET(au8SendBuf, 0, MAX_BODY_SIZE);
  MEM_CPY(au8SendBuf, pu8RxBuffer, u16DataLen + 1);
  
  //发送BUf命令初始化
  au8SendBuf[1] = 'S';
  au8SendBuf[2] = '1';
  au8SendBuf[3] = 'F';
  au8SendBuf[4] = 'K';
  
  pu8Temp = RealCmdProcess(au8SendBuf, S1PZ_CFG_CHK);
  pu8Temp++;
  //比对校验和是否正确
  if(u8CheckSum != *pu8Temp)
  {
    //发送状态赋值
    pu8Buf = RealCmdProcess(au8SendBuf, S1PZ_CFG_RESERVED);
    u8Temp = pu8Buf - au8SendBuf;
    au8SendBuf[u8Temp] = DTU_CFG_ERROR;
    
    //计算发送校验和
    u8CheckSum = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
    u8Temp = pu8Temp - au8SendBuf;
    au8SendBuf[u8Temp] = u8CheckSum;
    
    if(u8Index == USB_TYPE)
    {
      UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
      }
    }
    
    return CMD_S1PZ_ERROR;
  }
  else
  {
    //提取配置使能
    pu8Buf = RealCmdProcess(au8SendBuf, S1PZ_CFG_EN);
/*    if(*pu8Buf == DTU_CFG_VIEW)    //DTU不需要配置，中心查看DTU的信息
    {
      pu8Buf = RealCmdProcess(au8SendBuf, S1PZ_CFG_IP);
      u8Temp = pu8Buf - au8SendBuf;
      
      //获取IP地址, 若是域名，最大为61个byte
      i = CalcStringLen(&sDtuCfg[IP_SUB_1].au8BckIpAddr[0]);
      if(i>61) i = 61;
      MEM_CPY(&au8SendBuf[u8Temp], &sDtuCfg[IP_SUB_1].au8BckIpAddr[0], i);
      u8Temp += i;
      au8SendBuf[u8Temp++] = ',';
      
      //获取端口号
      au8SendBuf[u8Temp++] = sDtuCfg[IP_SUB_1].au8BckComIndex[0];
      au8SendBuf[u8Temp++] = sDtuCfg[IP_SUB_1].au8BckComIndex[1];
      au8SendBuf[u8Temp++] = ',';
      
      //发送状态
      au8SendBuf[u8Temp++] = DTU_CFG_OK;
      au8SendBuf[u8Temp++] = ',';
      
      au8SendBuf[u8Temp++] = '*';
      au8SendBuf[u8Temp++] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
      au8SendBuf[u8Temp]   = '\r';
      
      //获取数据长度位置
      pu8Buf = RealCmdProcess(au8SendBuf, S1PZ_CFG_LEN);
      pu8Buf++;
      i = pu8Buf - au8SendBuf;
      au8SendBuf[i]   = u8Temp;
      u16DataLen = u8Temp;
    }
    else
    {*/
      if((*pu8Buf == DTU_CFG_ENABLE) || (*pu8Buf == DTU_CFG_DISABLE))  //需要对分中心1进行配置
      {
        
        if(*pu8Buf == DTU_CFG_ENABLE)
        {
          u8CfgEnable = DTU_CFG_ENABLE;
        }
        else
        {
          u8CfgEnable = DTU_CFG_DISABLE;
        }
        pu8Buf = RealCmdProcess(au8SendBuf, S1PZ_CFG_RESERVED);
        u8Temp = pu8Buf - au8SendBuf;
        au8SendBuf[u8Temp] = DTU_CFG_OK;
        
        u8Temp = pu8Temp - au8SendBuf;
        au8SendBuf[u8Temp] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
        
        //获取数据长度位置
        pu8Buf = RealCmdProcess(au8SendBuf, S1PZ_CFG_LEN);
        pu8Buf++;
        i = pu8Buf - au8SendBuf;
        au8SendBuf[i]  = CalcDataLen(au8SendBuf, MAX_BODY_SIZE);
        u16DataLen = au8SendBuf[i];
        
        bSaved = TRUE;
      }
//    }
    
    //发送反馈信息
    if(u8Index == USB_TYPE)
    {
       UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
       g_u8TimerDelay = 0;
       while(g_u8TimerDelay < 10);
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
        g_u8TimerDelay = 0;
        while(g_u8TimerDelay < 10);
      }
    }
    
    if(bSaved == TRUE)
    {
      //清空主服务器配置缓冲区
     MEM_SET(&sDtuCfg[IP_SUB_1].au8CfgIpAddr[0], 0, (sizeof(_tsDtuCfg)/2));
      
      pu8Buf = RealCmdProcess(pu8RxBuffer, S1PZ_CFG_EN);
      sDtuCfg[IP_SUB_1].bCfgEnable = *pu8Buf;
      u8CfgEnable = *pu8Buf;
      
      //获得IP地址长度
      pu8Buf = RealCmdProcess(pu8RxBuffer, S1PZ_CFG_IP);
      pu8Temp = RealCmdProcess(pu8RxBuffer, S1PZ_CFG_COM);
      sDtuCfg[IP_SUB_1].u8CfgIpLen = pu8Temp - pu8Buf - 1;     //减去一个分隔符空间
      MEM_CPY(&sDtuCfg[IP_SUB_1].au8CfgIpAddr[0], pu8Buf, sDtuCfg[IP_SUB_1].u8CfgIpLen);
      
      //获得配置端口
      sDtuCfg[IP_SUB_1].au8CfgComIndex[0] = *pu8Temp++;
      sDtuCfg[IP_SUB_1].au8CfgComIndex[1] = *pu8Temp;
      
      if(0 != MEM_CMP(&sDtuCfg[IP_SUB_1].au8CfgIpAddr[0], &sDtuCfg[IP_SUB_1].au8BckIpAddr[0], (sizeof(_tsDtuCfg)/2)))
      { 
        MEM_SET(&sDtuCfg[IP_SUB_1].au8BckIpAddr[0], 0, (sizeof(_tsDtuCfg)/2));
        MEM_CPY(&sDtuCfg[IP_SUB_1].au8BckIpAddr[0], &sDtuCfg[IP_SUB_1].au8CfgIpAddr[0], (sizeof(_tsDtuCfg)/2));
        
        //关闭串口0和2
        UartDisableInterrupt(UART0);
        UartDisableInterrupt(UART2);
        
        u8State = McuConfigDtuSub1(u8CfgEnable);
        
        UartEnableInterrupt(UART0);
        UartEnableInterrupt(UART2);
        
        if(u8State == DTU_CFG_OK)
          return CMD_S1PZ_OK;
       else
       {
          pu8Buf = RealCmdProcess(au8SendBuf, S1PZ_CFG_RESERVED);
          u8Temp = pu8Buf - au8SendBuf;
          au8SendBuf[u8Temp] = u8State;   //配置失败，把具体的错误状态码再次发送给中心。
          
          u8Temp = pu8Temp - au8SendBuf;
          au8SendBuf[u8Temp] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
          
          //获取数据长度位置
          pu8Buf = RealCmdProcess(au8SendBuf, S1PZ_CFG_LEN);
          pu8Buf++;
          i = pu8Buf - au8SendBuf;
          au8SendBuf[i]   = CalcStringLen(au8SendBuf);
          
          //发送反馈信息
          if(u8Index == USB_TYPE)
          {
             UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
          }
          else
          {
            if(u8Index == DTU_TYPE)
            {
              UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
            }
          }
          
          return CMD_S1PZ_ERROR;
       }
      }
    }
  }
  
  return CMD_S1PZ_OK;
}

/**********************************************************************************************
 函数名称：Sub2ConfigProcess
 函数功能：DTU分中心1配置命令。

 参数： u8Index   需要解析的通道，USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 返回： 校验和状态
**********************************************************************************************/
UINT8 Sub2ConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer)
{
    UINT8 i;
  UINT8 *pu8Buf, *pu8Temp;
  UINT8 u8Temp;
  UINT16 u16DataLen=0;
  UINT8 u8CheckSum=0;
  UINT8 au8SendBuf[MAX_BODY_SIZE];
  UINT8 u8State = 0xff;
  BOOL bSaved = FALSE;
  UINT8 u8CfgEnable;
  
  if((u8Index != USB_TYPE) && (u8Index != DTU_TYPE))
  {
    return TYPE_ERROR;
  }
  
  //计算整个数据包的校验和，数据长度
  u8CheckSum = CalcCheckSum(pu8RxBuffer, MAX_BODY_SIZE);
  u16DataLen = CalcDataLen(pu8RxBuffer, MAX_BODY_SIZE);
    
  MEM_SET(au8SendBuf, 0, MAX_BODY_SIZE);
  MEM_CPY(au8SendBuf, pu8RxBuffer, u16DataLen + 1);
  
  //发送BUf命令初始化
  au8SendBuf[1] = 'S';
  au8SendBuf[2] = '2';
  au8SendBuf[3] = 'F';
  au8SendBuf[4] = 'K';
  
  pu8Temp = RealCmdProcess(au8SendBuf, S2PZ_CFG_CHK);
  pu8Temp++;
  //比对校验和是否正确
  if(u8CheckSum != *pu8Temp)
  {
    //发送状态赋值
    pu8Buf = RealCmdProcess(au8SendBuf, S2PZ_CFG_RESERVED);
    u8Temp = pu8Buf - au8SendBuf;
    au8SendBuf[u8Temp] = DTU_CFG_ERROR;
    
    //计算发送校验和
    u8CheckSum = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
    u8Temp = pu8Temp - au8SendBuf;
    au8SendBuf[u8Temp] = u8CheckSum;
    
    if(u8Index == USB_TYPE)
    {
      UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
      }
    }
    
    return CMD_S2PZ_ERROR;
  }
  else
  {
    //提取配置使能
    pu8Buf = RealCmdProcess(au8SendBuf, S2PZ_CFG_EN);
/*    if(*pu8Buf == DTU_CFG_VIEW)    //DTU不需要配置，中心查看DTU的信息
    {
      pu8Buf = RealCmdProcess(au8SendBuf, S2PZ_CFG_IP);
      u8Temp = pu8Buf - au8SendBuf;
      
      //获取IP地址, 若是域名，最大为61个byte
      i = CalcStringLen(&sDtuCfg[IP_SUB_2].au8BckIpAddr[0]);
      if(i>61) i = 61;
      MEM_CPY(&au8SendBuf[u8Temp], &sDtuCfg[IP_SUB_2].au8BckIpAddr[0], i);
      u8Temp += i;
      au8SendBuf[u8Temp++] = ',';
      
      //获取端口号
      au8SendBuf[u8Temp++] = sDtuCfg[IP_SUB_2].au8BckComIndex[0];
      au8SendBuf[u8Temp++] = sDtuCfg[IP_SUB_2].au8BckComIndex[1];
      au8SendBuf[u8Temp++] = ',';
      
      //发送状态
      au8SendBuf[u8Temp++] = DTU_CFG_OK;
      au8SendBuf[u8Temp++] = ',';
      
      au8SendBuf[u8Temp++] = '*';
      au8SendBuf[u8Temp++] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
      au8SendBuf[u8Temp]   = '\r';
      
      //获取数据长度位置
      pu8Buf = RealCmdProcess(au8SendBuf, S2PZ_CFG_LEN);
      pu8Buf++;
      i = pu8Buf - au8SendBuf;
      au8SendBuf[i]   = u8Temp;
      u16DataLen = u8Temp;
    }
    else
    {*/
      if((*pu8Buf == DTU_CFG_ENABLE) || (*pu8Buf == DTU_CFG_DISABLE))  //需要对分中心1进行配置
      {
        
        if(*pu8Buf == DTU_CFG_ENABLE)
        {
          u8CfgEnable = DTU_CFG_ENABLE;
        }
        else
        {
          u8CfgEnable = DTU_CFG_DISABLE;
        }
        pu8Buf = RealCmdProcess(au8SendBuf, S2PZ_CFG_RESERVED);
        u8Temp = pu8Buf - au8SendBuf;
        au8SendBuf[u8Temp] = DTU_CFG_OK;
        
        u8Temp = pu8Temp - au8SendBuf;
        au8SendBuf[u8Temp] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
        
        //获取数据长度位置
        pu8Buf = RealCmdProcess(au8SendBuf, S2PZ_CFG_LEN);
        pu8Buf++;
        i = pu8Buf - au8SendBuf;
        au8SendBuf[i]  = CalcDataLen(au8SendBuf, MAX_BODY_SIZE);
        u16DataLen = au8SendBuf[i];
        
        bSaved = TRUE;
      }
//    }
    
    //发送反馈信息
    if(u8Index == USB_TYPE)
    {
       UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
       g_u8TimerDelay = 0;
        while(g_u8TimerDelay < 10);
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
        g_u8TimerDelay = 0;
        while(g_u8TimerDelay < 10);
      }
    }
    
    if(bSaved == TRUE)
    {
      //清空主服务器配置缓冲区
      MEM_SET(&sDtuCfg[IP_SUB_2].au8CfgIpAddr[0], 0, (sizeof(_tsDtuCfg)/2));
      
      pu8Buf = RealCmdProcess(pu8RxBuffer, S2PZ_CFG_EN);
      sDtuCfg[IP_SUB_2].bCfgEnable = *pu8Buf;
      u8CfgEnable = *pu8Buf;
      
      //获得IP地址长度
      pu8Buf = RealCmdProcess(pu8RxBuffer, S2PZ_CFG_IP);
      pu8Temp = RealCmdProcess(pu8RxBuffer, S2PZ_CFG_COM);
      sDtuCfg[IP_SUB_2].u8CfgIpLen = pu8Temp - pu8Buf - 1;     //减去一个分隔符空间
      MEM_CPY(&sDtuCfg[IP_SUB_2].au8CfgIpAddr[0], pu8Buf, sDtuCfg[IP_SUB_2].u8CfgIpLen);
      
      //获得配置端口
      sDtuCfg[IP_SUB_2].au8CfgComIndex[0] = *pu8Temp++;
      sDtuCfg[IP_SUB_2].au8CfgComIndex[1] = *pu8Temp;
      
      if(0 != MEM_CMP(&sDtuCfg[IP_SUB_2].au8CfgIpAddr[0], &sDtuCfg[IP_SUB_2].au8BckIpAddr[0], (sizeof(_tsDtuCfg)/2)))
      { 
        MEM_SET(&sDtuCfg[IP_SUB_2].au8BckIpAddr[0], 0, (sizeof(_tsDtuCfg)/2));
        MEM_CPY(&sDtuCfg[IP_SUB_2].au8BckIpAddr[0], &sDtuCfg[IP_SUB_2].au8CfgIpAddr[0], (sizeof(_tsDtuCfg)/2));
        
        //关闭串口0和2
        UartDisableInterrupt(UART0);
        UartDisableInterrupt(UART2);
        
        u8State = McuConfigDtuSub2(u8CfgEnable);
        
        UartEnableInterrupt(UART0);
        UartEnableInterrupt(UART2);
        
        if(u8State == DTU_CFG_OK)
          return CMD_S2PZ_OK;
       else
       {
          pu8Buf = RealCmdProcess(au8SendBuf, S2PZ_CFG_RESERVED);
          u8Temp = pu8Buf - au8SendBuf;
          au8SendBuf[u8Temp] = u8State;   //配置失败，把具体的错误状态码再次发送给中心。
          
          u8Temp = pu8Temp - au8SendBuf;
          au8SendBuf[u8Temp] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
          
          //获取数据长度位置
          pu8Buf = RealCmdProcess(au8SendBuf, S2PZ_CFG_LEN);
          pu8Buf++;
          i = pu8Buf - au8SendBuf;
          au8SendBuf[i]   = CalcStringLen(au8SendBuf);
          
          //发送反馈信息
          if(u8Index == USB_TYPE)
          {
             UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
          }
          else
          {
            if(u8Index == DTU_TYPE)
            {
              UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
            }
          }
          
          return CMD_S2PZ_ERROR;
       }
      }
    }
  }
  
  return CMD_S2PZ_OK;
}

/**********************************************************************************************
 函数名称：Sub3ConfigProcess
 函数功能：DTU分中心1配置命令。

 参数： u8Index   需要解析的通道，USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 返回： 校验和状态
**********************************************************************************************/
UINT8 Sub3ConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer)
{
  UINT8 i;
  UINT8 *pu8Buf, *pu8Temp;
  UINT8 u8Temp;
  UINT16 u16DataLen=0;
  UINT8 u8CheckSum=0;
  UINT8 au8SendBuf[MAX_BODY_SIZE];
  UINT8 u8State = 0xff;
  BOOL bSaved = FALSE;
  UINT8 u8CfgEnable;
  
  if((u8Index != USB_TYPE) && (u8Index != DTU_TYPE))
  {
    return TYPE_ERROR;
  }
  
  //计算整个数据包的校验和，数据长度
  u8CheckSum = CalcCheckSum(pu8RxBuffer, MAX_BODY_SIZE);
  u16DataLen = CalcDataLen(pu8RxBuffer, MAX_BODY_SIZE);
    
  MEM_SET(au8SendBuf, 0, MAX_BODY_SIZE);
  MEM_CPY(au8SendBuf, pu8RxBuffer, u16DataLen + 1);
  
  //发送BUf命令初始化
  au8SendBuf[1] = 'S';
  au8SendBuf[2] = '3';
  au8SendBuf[3] = 'F';
  au8SendBuf[4] = 'K';
  
  pu8Temp = RealCmdProcess(au8SendBuf, S3PZ_CFG_CHK);
  pu8Temp++;
  //比对校验和是否正确
  if(u8CheckSum != *pu8Temp)
  {
    //发送状态赋值
    pu8Buf = RealCmdProcess(au8SendBuf, S3PZ_CFG_RESERVED);
    u8Temp = pu8Buf - au8SendBuf;
    au8SendBuf[u8Temp] = DTU_CFG_ERROR;
    
    //计算发送校验和
    u8CheckSum = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
    u8Temp = pu8Temp - au8SendBuf;
    au8SendBuf[u8Temp] = u8CheckSum;
    
    if(u8Index == USB_TYPE)
    {
      UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
      }
    }
    
    return CMD_S3PZ_ERROR;
  }
  else
  {
    //提取配置使能
    pu8Buf = RealCmdProcess(au8SendBuf, S3PZ_CFG_EN);
/*    if(*pu8Buf == DTU_CFG_VIEW)    //DTU不需要配置，中心查看DTU的信息
    {
      pu8Buf = RealCmdProcess(au8SendBuf, S3PZ_CFG_IP);
      u8Temp = pu8Buf - au8SendBuf;
      
      //获取IP地址, 若是域名，最大为61个byte
      i = CalcStringLen(&sDtuCfg[IP_SUB_3].au8BckIpAddr[0]);
      if(i>61) i = 61;
      MEM_CPY(&au8SendBuf[u8Temp], &sDtuCfg[IP_SUB_3].au8BckIpAddr[0], i);
      u8Temp += i;
      au8SendBuf[u8Temp++] = ',';
      
      //获取端口号
      au8SendBuf[u8Temp++] = sDtuCfg[IP_SUB_3].au8BckComIndex[0];
      au8SendBuf[u8Temp++] = sDtuCfg[IP_SUB_3].au8BckComIndex[1];
      au8SendBuf[u8Temp++] = ',';
      
      //发送状态
      au8SendBuf[u8Temp++] = DTU_CFG_OK;
      au8SendBuf[u8Temp++] = ',';
      
      au8SendBuf[u8Temp++] = '*';
      au8SendBuf[u8Temp++] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
      au8SendBuf[u8Temp]   = '\r';
      
      //获取数据长度位置
      pu8Buf = RealCmdProcess(au8SendBuf, S3PZ_CFG_LEN);
      pu8Buf++;
      i = pu8Buf - au8SendBuf;
      au8SendBuf[i]   = u8Temp;
      u16DataLen = u8Temp;
    }
    else
   {*/
      if((*pu8Buf == DTU_CFG_ENABLE) || (*pu8Buf == DTU_CFG_DISABLE))  //需要对分中心1进行配置
      {
        
        if(*pu8Buf == DTU_CFG_ENABLE)
        {
          u8CfgEnable = DTU_CFG_ENABLE;
        }
        else
        {
          u8CfgEnable = DTU_CFG_DISABLE;
        }
        
        pu8Buf = RealCmdProcess(au8SendBuf, S3PZ_CFG_RESERVED);
        u8Temp = pu8Buf - au8SendBuf;
        au8SendBuf[u8Temp] = DTU_CFG_OK;
        
        u8Temp = pu8Temp - au8SendBuf;
        au8SendBuf[u8Temp] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
        
        //获取数据长度位置
        pu8Buf = RealCmdProcess(au8SendBuf, S3PZ_CFG_LEN);
        pu8Buf++;
        i = pu8Buf - au8SendBuf;
        au8SendBuf[i]  = CalcDataLen(au8SendBuf, MAX_BODY_SIZE);
        u16DataLen = au8SendBuf[i];
        
        bSaved = TRUE;
      }
 //   }
    
    //发送反馈信息
    if(u8Index == USB_TYPE)
    {
       UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
       g_u8TimerDelay = 0;
        while(g_u8TimerDelay < 10);
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
        g_u8TimerDelay = 0;
        while(g_u8TimerDelay < 10);
      }
    }
    
    if(bSaved == TRUE)
    {
      //清空主服务器配置缓冲区
      MEM_SET(&sDtuCfg[IP_SUB_3].au8CfgIpAddr[0], 0, (sizeof(_tsDtuCfg)/2));
      
      pu8Buf = RealCmdProcess(pu8RxBuffer, S3PZ_CFG_EN);
      sDtuCfg[IP_SUB_3].bCfgEnable = *pu8Buf;
      u8CfgEnable = *pu8Buf;
      
      //获得IP地址长度
      pu8Buf = RealCmdProcess(pu8RxBuffer, S3PZ_CFG_IP);
      pu8Temp = RealCmdProcess(pu8RxBuffer, S3PZ_CFG_COM);
      sDtuCfg[IP_SUB_3].u8CfgIpLen = pu8Temp - pu8Buf - 1;     //减去一个分隔符空间
      MEM_CPY(&sDtuCfg[IP_SUB_3].au8CfgIpAddr[0], pu8Buf, sDtuCfg[IP_SUB_3].u8CfgIpLen);
      
      //获得配置端口
      sDtuCfg[IP_SUB_3].au8CfgComIndex[0] = *pu8Temp++;
      sDtuCfg[IP_SUB_3].au8CfgComIndex[1] = *pu8Temp;
      
      if(0 != MEM_CMP(&sDtuCfg[IP_SUB_1].au8CfgIpAddr[0], &sDtuCfg[IP_SUB_3].au8BckIpAddr[0], (sizeof(_tsDtuCfg)/2)))
      { 
        MEM_SET(&sDtuCfg[IP_SUB_3].au8BckIpAddr[0], 0, (sizeof(_tsDtuCfg)/2));
        MEM_CPY(&sDtuCfg[IP_SUB_3].au8BckIpAddr[0], &sDtuCfg[IP_SUB_3].au8CfgIpAddr[0], (sizeof(_tsDtuCfg)/2));
        
        //关闭串口0和2
        UartDisableInterrupt(UART0);
        UartDisableInterrupt(UART2);
        
        u8State = McuConfigDtuSub3(u8CfgEnable);
        
        UartEnableInterrupt(UART0);
        UartEnableInterrupt(UART2);
        
        if(u8State == DTU_CFG_OK)
          return CMD_S3PZ_OK;
       else
       {
          pu8Buf = RealCmdProcess(au8SendBuf, S3PZ_CFG_RESERVED);
          u8Temp = pu8Buf - au8SendBuf;
          au8SendBuf[u8Temp] = u8State;   //配置失败，把具体的错误状态码再次发送给中心。
          
          u8Temp = pu8Temp - au8SendBuf;
          au8SendBuf[u8Temp] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
          
          //获取数据长度位置
          pu8Buf = RealCmdProcess(au8SendBuf, S3PZ_CFG_LEN);
          pu8Buf++;
          i = pu8Buf - au8SendBuf;
          au8SendBuf[i]   = CalcStringLen(au8SendBuf);
          
          //发送反馈信息
          if(u8Index == USB_TYPE)
          {
             UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
          }
          else
          {
            if(u8Index == DTU_TYPE)
            {
              UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
            }
          }
          
          return CMD_S3PZ_ERROR;
       }
      }
    }
  }
  
  return CMD_S3PZ_OK;
}

/**********************************************************************************************
 函数名称：Sub4ConfigProcess
 函数功能：DTU分中心1配置命令。

 参数： u8Index   需要解析的通道，USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 返回： 校验和状态
**********************************************************************************************/
UINT8 Sub4ConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer)
{
  UINT8 i;
  UINT8 *pu8Buf, *pu8Temp;
  UINT8 u8Temp;
  UINT16 u16DataLen=0;
  UINT8 u8CheckSum=0;
  UINT8 au8SendBuf[MAX_BODY_SIZE];
  UINT8 u8State = 0xff;
  BOOL bSaved = FALSE;
  UINT8 u8CfgEnable;
  
  if((u8Index != USB_TYPE) && (u8Index != DTU_TYPE))
  {
    return TYPE_ERROR;
  }
  
  //计算整个数据包的校验和，数据长度
  u8CheckSum = CalcCheckSum(pu8RxBuffer, MAX_BODY_SIZE);
  u16DataLen = CalcDataLen(pu8RxBuffer, MAX_BODY_SIZE);
    
  MEM_SET(au8SendBuf, 0, MAX_BODY_SIZE);
  MEM_CPY(au8SendBuf, pu8RxBuffer, u16DataLen + 1);
  
  //发送BUf命令初始化
  au8SendBuf[1] = 'S';
  au8SendBuf[2] = '4';
  au8SendBuf[3] = 'F';
  au8SendBuf[4] = 'K';
  
  pu8Temp = RealCmdProcess(au8SendBuf, S4PZ_CFG_CHK);
  pu8Temp++;
  //比对校验和是否正确
  if(u8CheckSum != *pu8Temp)
  {
    //发送状态赋值
    pu8Buf = RealCmdProcess(au8SendBuf, S4PZ_CFG_RESERVED);
    u8Temp = pu8Buf - au8SendBuf;
    au8SendBuf[u8Temp] = DTU_CFG_ERROR;
    
    //计算发送校验和
    u8CheckSum = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
    u8Temp = pu8Temp - au8SendBuf;
    au8SendBuf[u8Temp] = u8CheckSum;
    
    if(u8Index == USB_TYPE)
    {
      UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
      }
    }
    
    return CMD_S4PZ_ERROR;
  }
  else
  {
    //提取配置使能
    pu8Buf = RealCmdProcess(au8SendBuf, S4PZ_CFG_EN);
/*    if(*pu8Buf == DTU_CFG_VIEW)    //DTU不需要配置，中心查看DTU的信息
    {
      pu8Buf = RealCmdProcess(au8SendBuf, S4PZ_CFG_IP);
      u8Temp = pu8Buf - au8SendBuf;
      
      //获取IP地址, 若是域名，最大为61个byte
      i = CalcStringLen(&sDtuCfg[IP_SUB_4].au8BckIpAddr[0]);
      if(i>61) i = 61;
      MEM_CPY(&au8SendBuf[u8Temp], &sDtuCfg[IP_SUB_4].au8BckIpAddr[0], i);
      u8Temp += i;
      au8SendBuf[u8Temp++] = ',';
      
      //获取端口号
      au8SendBuf[u8Temp++] = sDtuCfg[IP_SUB_4].au8BckComIndex[0];
      au8SendBuf[u8Temp++] = sDtuCfg[IP_SUB_4].au8BckComIndex[1];
      au8SendBuf[u8Temp++] = ',';
      
      //发送状态
      au8SendBuf[u8Temp++] = DTU_CFG_OK;
      au8SendBuf[u8Temp++] = ',';
      
      au8SendBuf[u8Temp++] = '*';
      au8SendBuf[u8Temp++] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
      au8SendBuf[u8Temp]   = '\r';
      
      //获取数据长度位置
      pu8Buf = RealCmdProcess(au8SendBuf, S4PZ_CFG_LEN);
      pu8Buf++;
      i = pu8Buf - au8SendBuf;
      au8SendBuf[i]   = u8Temp;
      u16DataLen = u8Temp;
    }
    else
    {*/
      if((*pu8Buf == DTU_CFG_ENABLE) || (*pu8Buf == DTU_CFG_DISABLE))  //需要对分中心1进行配置
      {
        
        if(*pu8Buf == DTU_CFG_ENABLE)
        {
          u8CfgEnable = DTU_CFG_ENABLE;
        }
        else
        {
          u8CfgEnable = DTU_CFG_DISABLE;
        }
        pu8Buf = RealCmdProcess(au8SendBuf, S4PZ_CFG_RESERVED);
        u8Temp = pu8Buf - au8SendBuf;
        au8SendBuf[u8Temp] = DTU_CFG_OK;
        
        u8Temp = pu8Temp - au8SendBuf;
        au8SendBuf[u8Temp] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
        
        //获取数据长度位置
        pu8Buf = RealCmdProcess(au8SendBuf, S4PZ_CFG_LEN);
        pu8Buf++;
        i = pu8Buf - au8SendBuf;
        au8SendBuf[i]  = CalcDataLen(au8SendBuf, MAX_BODY_SIZE);
        u16DataLen = au8SendBuf[i];
        
        bSaved = TRUE;
      }
//    }
    
    //发送反馈信息
    if(u8Index == USB_TYPE)
    {
       UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
       g_u8TimerDelay = 0;
        while(g_u8TimerDelay < 10);
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
        g_u8TimerDelay = 0;
        while(g_u8TimerDelay < 10);
      }
    }
    
    if(bSaved == TRUE)
    {
      //清空主服务器配置缓冲区
      MEM_SET(&sDtuCfg[IP_SUB_4].au8CfgIpAddr[0], 0, (sizeof(_tsDtuCfg)/2));
      
      pu8Buf = RealCmdProcess(pu8RxBuffer, S4PZ_CFG_EN);
      sDtuCfg[IP_SUB_4].bCfgEnable = *pu8Buf;
      u8CfgEnable = *pu8Buf;
      
      //获得IP地址长度
      pu8Buf = RealCmdProcess(pu8RxBuffer, S4PZ_CFG_IP);
      pu8Temp = RealCmdProcess(pu8RxBuffer, S4PZ_CFG_COM);
      sDtuCfg[IP_SUB_4].u8CfgIpLen = pu8Temp - pu8Buf - 1;     //减去一个分隔符空间
      MEM_CPY(&sDtuCfg[IP_SUB_4].au8CfgIpAddr[0], pu8Buf, sDtuCfg[IP_SUB_4].u8CfgIpLen);
      
      //获得配置端口
      sDtuCfg[IP_SUB_4].au8CfgComIndex[0] = *pu8Temp++;
      sDtuCfg[IP_SUB_4].au8CfgComIndex[1] = *pu8Temp;
      
      if(0 != MEM_CMP(&sDtuCfg[IP_SUB_4].au8CfgIpAddr[0], &sDtuCfg[IP_SUB_4].au8BckIpAddr[0], (sizeof(_tsDtuCfg)/2)))
      { 
        MEM_SET(&sDtuCfg[IP_SUB_4].au8BckIpAddr[0], 0, (sizeof(_tsDtuCfg)/2));
        MEM_CPY(&sDtuCfg[IP_SUB_4].au8BckIpAddr[0], &sDtuCfg[IP_SUB_4].au8CfgIpAddr[0], (sizeof(_tsDtuCfg)/2));
        
        //关闭串口0和2
        UartDisableInterrupt(UART0);
        UartDisableInterrupt(UART2);
        
        u8State = McuConfigDtuSub4(u8CfgEnable);
        
        UartEnableInterrupt(UART0);
        UartEnableInterrupt(UART2);
        
        if(u8State == DTU_CFG_OK)
          return CMD_S4PZ_OK;
       else
       {
          pu8Buf = RealCmdProcess(au8SendBuf, S4PZ_CFG_RESERVED);
          u8Temp = pu8Buf - au8SendBuf;
          au8SendBuf[u8Temp] = u8State;   //配置失败，把具体的错误状态码再次发送给中心。
          
          u8Temp = pu8Temp - au8SendBuf;
          au8SendBuf[u8Temp] = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
          
          //获取数据长度位置
          pu8Buf = RealCmdProcess(au8SendBuf, S4PZ_CFG_LEN);
          pu8Buf++;
          i = pu8Buf - au8SendBuf;
          au8SendBuf[i]   = CalcStringLen(au8SendBuf);
          
          //发送反馈信息
          if(u8Index == USB_TYPE)
          {
             UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
          }
          else
          {
            if(u8Index == DTU_TYPE)
            {
              UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
            }
          }
          
          return CMD_S4PZ_ERROR;
       }
      }
    }
  }
  
  return CMD_S4PZ_OK;
}

/**********************************************************************************************
 函数名称： DebugProcess
 函数功能：debug接口开启关闭处理。

 参数： u8Index   需要解析的通道，USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 返回： 校验和状态
**********************************************************************************************/
UINT8 DebugProcess(UINT8 u8Index, UINT8 *pu8RxBuffer)
{
  UINT16 u16DataLen=0;
  UINT8 u8CheckSum=0;
  UINT8 au8SendBuf[20];

  if((u8Index != USB_TYPE) && (u8Index != DTU_TYPE))
  {
    return TYPE_ERROR;
  }
  
  //计算整个数据包的校验和，数据长度
  u8CheckSum = CalcCheckSum(pu8RxBuffer, MAX_BODY_SIZE);
  u16DataLen = CalcDataLen(pu8RxBuffer, MAX_BODY_SIZE);
    
  MEM_SET(au8SendBuf, 0, 20);
  MEM_CPY(au8SendBuf, pu8RxBuffer, u16DataLen + 1);
  
  //发送BUf命令初始化
  au8SendBuf[1] = 'T';
  au8SendBuf[2] = 'S';
  au8SendBuf[3] = 'K';
  au8SendBuf[4] = 'G';
  
  //比对校验和是否正确
  if(u8CheckSum != pu8RxBuffer[9])
  {
    //发送状态赋值
    au8SendBuf[6] = 0x03;
    
    //计算发送校验和
    au8SendBuf[9] = CalcCheckSum(au8SendBuf, 20);
    
    if(u8Index == USB_TYPE)
    {
      UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
      }
    }
    
    return CMD_TSKG_ERROR;
  }
  else
  {
    if((au8SendBuf[6] == 0) || (au8SendBuf[6] == 1))
    {
      if(au8SendBuf[6] != g_bDebug)
      {
        g_bDebug = au8SendBuf[6];
        g_u8NeedSaved |= SAVE_DEBUG_SW;  
      }
    
      u8CheckSum = CalcCheckSum(au8SendBuf, MAX_BODY_SIZE);
      
       //发送反馈信息
      if(u8Index == USB_TYPE)
      {
         UartSendBuffer(UART2, &au8SendBuf[0], (u16DataLen+1));
      }
      else
      {
        if(u8Index == DTU_TYPE)
        {
          UartSendBuffer(UART1, &au8SendBuf[0], (u16DataLen+1));
        }
      }
      
      return CMD_TSKG_OK;
    }
  }
return CMD_TSKG_ERROR;
}


/**********************************************************************************************
 函数名称： GetVersionProcess
 函数功能：获取版本信息

 参数： u8Index   需要解析的通道，USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 返回： 校验和状态
**********************************************************************************************/
UINT8 GetVersionProcess(UINT8 u8Index, UINT8 *pu8RxBuffer)
{
  UINT8 i;
  UINT16 u16DataLen=0;
  UINT8 u8CheckSum=0;
  UINT8 au8SendBuf[20];

  if((u8Index != USB_TYPE) && (u8Index != DTU_TYPE))
  {
    return TYPE_ERROR;
  }
  
  //计算整个数据包的校验和，数据长度
  //计算发送校验和
  for(i=1; i<20; i++)
  {
    if(pu8RxBuffer[i] != '*')
    {
      u8CheckSum ^= pu8RxBuffer[i];
    }
    else
    {
      i++;
      break;
    }
  }

  u16DataLen = i+1;
    
  MEM_SET(au8SendBuf, 0, 20);
  MEM_CPY(au8SendBuf, pu8RxBuffer, u16DataLen + 1);
  
  //发送BUf命令初始化
  au8SendBuf[1] = 'B';
  au8SendBuf[2] = 'B';
  au8SendBuf[3] = 'H';
  au8SendBuf[4] = 'M';
  
  //比对校验和是否正确
  if(u8CheckSum != pu8RxBuffer[i])
  {
    //发送状态赋值
    au8SendBuf[5] = ',';
    au8SendBuf[6] = 0;
    au8SendBuf[7] = 13;
    au8SendBuf[8] = ',';
    au8SendBuf[9] = 0x00;
    au8SendBuf[10] = ',';
    au8SendBuf[11] = '*';
    
    //计算发送校验和
    au8SendBuf[12] = CalcCheckSum(au8SendBuf, 20);
    au8SendBuf[13] = 0x0D;
    
    if(u8Index == USB_TYPE)
    {
      UartSendBuffer(UART2, &au8SendBuf[0], 14);
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], 14);
      }
    }
    
    return CMD_BBHM_ERROR;
  }
  else
  {
     au8SendBuf[5] = ',';
     au8SendBuf[6] = 0;
     au8SendBuf[7] = 14;
     au8SendBuf[8] = ',';
     au8SendBuf[9] = VERSION_MAIN;
     au8SendBuf[10] = VERSION_SUB;
     au8SendBuf[11] = ',';
     au8SendBuf[12] = '*';
    
     au8SendBuf[13] = CalcCheckSum(au8SendBuf, 20);
     au8SendBuf[14] = 0x0d;
      
       //发送反馈信息
     if(u8Index == USB_TYPE)
     {
        UartSendBuffer(UART2, &au8SendBuf[0], 15);
     }
     else
     {
       if(u8Index == DTU_TYPE)
       {
         UartSendBuffer(UART1, &au8SendBuf[0], 15);
       }
     }
      
     return CMD_BBHM_OK;
  }

}

/**********************************************************************************************
 函数名称： PowerDownProcess
 函数功能：终端关机指令

 参数： u8Index   需要解析的通道，USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 返回： 校验和状态
**********************************************************************************************/
UINT8 PowerDownProcess(UINT8 u8Index, UINT8 *pu8RxBuffer)
{
  UINT8 i;
  UINT16 u16DataLen=0;
  UINT8 u8CheckSum=0;
  UINT8 au8SendBuf[20];

  if((u8Index != USB_TYPE) && (u8Index != DTU_TYPE))
  {
    return TYPE_ERROR;
  }
  
  //计算整个数据包的校验和，数据长度
  //计算发送校验和
  for(i=1; i<20; i++)
  {
    if(pu8RxBuffer[i] != '*')
    {
      u8CheckSum ^= pu8RxBuffer[i];
    }
    else
    {
      i++;
      break;
    }
  }

  u16DataLen = i+1;
    
  MEM_SET(au8SendBuf, 0, 20);
  MEM_CPY(au8SendBuf, pu8RxBuffer, u16DataLen + 1);
  
  //发送BUf命令初始化
  au8SendBuf[1] = 'Z';
  au8SendBuf[2] = 'D';
  au8SendBuf[3] = 'G';
  au8SendBuf[4] = 'J';
  
  //比对校验和是否正确
  if(u8CheckSum != pu8RxBuffer[i])
  {
    //发送状态赋值
    au8SendBuf[5] = ',';
    au8SendBuf[6] = 0;
    au8SendBuf[7] = 13;
    au8SendBuf[8] = ',';
    au8SendBuf[9] = 0x00;
    au8SendBuf[10] = ',';
    au8SendBuf[11] = '*';
    
    //计算发送校验和
    au8SendBuf[12] = CalcCheckSum(au8SendBuf, 20);
    au8SendBuf[13] = 0x0D;
    
    if(u8Index == USB_TYPE)
    {
      UartSendBuffer(UART2, &au8SendBuf[0], 14);
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], 14);
      }
    }
    
    return CMD_ZDGJ_ERROR;
  }
  else
  {
     au8SendBuf[5] = ',';
     au8SendBuf[6] = 0;
     au8SendBuf[7] = 13;
     au8SendBuf[8] = ',';
     au8SendBuf[9] = 0x01;
     au8SendBuf[10] = ',';
     au8SendBuf[11] = '*';
    
     au8SendBuf[12] = CalcCheckSum(au8SendBuf, 20);
     au8SendBuf[13] = 0x0d;
      
       //发送反馈信息
     if(u8Index == USB_TYPE)
     {
        UartSendBuffer(UART2, &au8SendBuf[0], 14);
     }
     else
     {
       if(u8Index == DTU_TYPE)
       {
         UartSendBuffer(UART1, &au8SendBuf[0], 14);
       }
     }
     
     //显示关机画面
    DisplayOffPage();
    //关闭DTU电源
    DtuPowerOff();
    GpsPowerOff();
    //关闭OLED电源
    OledPowerDown();
    //关闭系统电源
    SystemPowerOff();
    OpenWatchDog(); // 打开看门狗
    while(1);
      
//     return CMD_ZDGJ_OK;
  }

}

/**********************************************************************************************
 函数名称： ReadDtuConfigProcess
 函数功能：获取DTU的标示符

 参数： u8Index   需要解析的通道，USB_CHANNEL, DTU_CHANNEL
        *pu8RxBuffer
 返回： 校验和状态
**********************************************************************************************/
UINT8 ReadDtuConfigProcess(UINT8 u8Index, UINT8 *pu8RxBuffer)
{
   UINT8 i;
  UINT16 u16DataLen=0;
  UINT8 u8CheckSum=0;
  UINT8 au8SendBuf[24];

  if((u8Index != USB_TYPE) && (u8Index != DTU_TYPE))
  {
    return TYPE_ERROR;
  }
  
  //计算整个数据包的校验和，数据长度
  //计算发送校验和
  for(i=1; i<20; i++)
  {
    if(pu8RxBuffer[i] != '*')
    {
      u8CheckSum ^= pu8RxBuffer[i];
    }
    else
    {
      i++;
      break;
    }
  }

  u16DataLen = i+1;
    
  MEM_SET(au8SendBuf, 0, 20);
  MEM_CPY(au8SendBuf, pu8RxBuffer, u16DataLen + 1);
  
  //发送BUf命令初始化
  au8SendBuf[1] = 'Y';
  au8SendBuf[2] = 'J';
  au8SendBuf[3] = 'X';
  au8SendBuf[4] = 'X';
  
  //比对校验和是否正确
  if(u8CheckSum != pu8RxBuffer[i])
  {
    au8SendBuf[5] = ',';
    au8SendBuf[6] = 0;
    au8SendBuf[7] = 13;
    //发送状态赋值
    au8SendBuf[8] = ',';
    au8SendBuf[9] = 0x00;
    au8SendBuf[10] = ',';
    au8SendBuf[11] = '*';
    
    //计算发送校验和
    au8SendBuf[12] = CalcCheckSum(au8SendBuf, 20);
    au8SendBuf[13] = 0x0D;
    
    if(u8Index == USB_TYPE)
    {
      UartSendBuffer(UART2, &au8SendBuf[0], 14);
    }
    else
    {
      if(u8Index == DTU_TYPE)
      {
        UartSendBuffer(UART1, &au8SendBuf[0], 14);
      }
    }
    
    return CMD_DTHM_ERROR;
  }
  else
  {
     au8SendBuf[5] = ',';
     au8SendBuf[6] = 0;
     au8SendBuf[7] = 23;
     au8SendBuf[8] = ',';
     MEM_CPY(&au8SendBuf[9], &sMainServer.au8BckId[0], 11);
     au8SendBuf[20] = ',';
     au8SendBuf[21] = '*';
    
     au8SendBuf[22] = CalcCheckSum(au8SendBuf, 20);
     au8SendBuf[23] = 0x0d;
      
       //发送反馈信息
     if(u8Index == USB_TYPE)
     {
        UartSendBuffer(UART2, &au8SendBuf[0], 24);
     }
     else
     {
       if(u8Index == DTU_TYPE)
       {
         UartSendBuffer(UART1, &au8SendBuf[0], 24);
       }
     }
     

     return CMD_DTHM_OK;
  }

}

/**********************************************************************************************
 函数名称：ConfigPackType
 函数功能：配置命令解析

 参数： 无
 返回： 命令类型
**********************************************************************************************/
SINT8 ConfigPackType(const UINT8 *pu8Buf, UINT8 u8Size)
{
  if(u8Size < 5)
    return PZNON;
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[0], 5))
    return XXPZ;    
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[1], 5))
    return DMPZ;
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[2], 5))
    return S1PZ;
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[3], 5))
    return S2PZ;
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[4], 5))
    return S3PZ;
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[5], 5))
    return S4PZ;
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[6], 5))
    return ZDQD;
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[7], 5))
    return ZDTZ;
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[8], 5))
    return ZDGJ;
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[9], 5))
    return TSKG;
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[10], 5))
    return BBHM;
  else if(0 == MEM_CMP(pu8Buf, pCfgCmd[11], 5))
    return YJCX;

  return PZNON;
}
/* ====================================================================================================== */

/***************************************************************************
//函数名称：UsbCmdInit
//功能描述：USB转串口命令解析初始化
//
//参数：无
//返回：无
***************************************************************************/
void UsbCmdInit(void)
{
  sUartCmdCb[USB_TYPE].eAssembleState = E_UART_CMD_RECEIVING_HEADER;
  sUartCmdCb[USB_TYPE].pu8AssembleBuf = (UINT8 *)au8UsbBodyBuf;
  sUartCmdCb[USB_TYPE].u8BufDataLength = 0;

  //串口0接收超时定时器。
  asTimer[TIMER_UART2].u16LifeTime = TIMER_TICK / 2;
  asTimer[TIMER_UART2].u16LifeOldTime = TIMER_TICK / 2;
  asTimer[TIMER_UART2].bEnable = FALSE;
}

/***********************************************************************************************
  函数名称：UartCmdAssemble(UINT8 u8InData)
  函数功能：把接收的数据组合成有效的数据包

  输入：u8InData
  输出：无
***********************************************************************************************/
void Uart2CmdAssemble(UINT8 u8InData)
{
  switch(sUartCmdCb[USB_TYPE].eAssembleState)
  {
    case E_UART_CMD_RECEIVING_HEADER:
    {
      if(u8InData == '$')
      {
        sUartCmdCb[USB_TYPE].pu8AssembleBuf[0] = u8InData;
        sUartCmdCb[USB_TYPE].u8BufDataLength = 1;
	      sUartCmdCb[USB_TYPE].eAssembleState = E_UART_CMD_RECEIVING_BODY;	
      }
      break;
     }	
		
    case E_UART_CMD_RECEIVING_BODY:
    {
      sUartCmdCb[USB_TYPE].pu8AssembleBuf[sUartCmdCb[USB_TYPE].u8BufDataLength++] = u8InData;
      if((u8InData == '\r') && (sUartCmdCb[USB_TYPE].pu8AssembleBuf[sUartCmdCb[USB_TYPE].u8BufDataLength-3] == '*'))
      {
        sUartCmdCb[USB_TYPE].eAssembleState = E_UART_CMD_RECEIVING_VALID;
      }
      else
      {
        if(sUartCmdCb[USB_TYPE].u8BufDataLength == MAX_BODY_SIZE)	
        {
          sUartCmdCb[USB_TYPE].eAssembleState = E_UART_CMD_RECEIVING_HEADER;
          sUartCmdCb[USB_TYPE].u8BufDataLength = 0;
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
      sUartCmdCb[USB_TYPE].eAssembleState = E_UART_CMD_RECEIVING_HEADER;
      sUartCmdCb[USB_TYPE].u8BufDataLength = 0;
    }
  }	
}


/***************************************************************************
//函数名称：UsbCmdRun
//功能描述：USB转串口命令解析运行
//
//参数：无
//返回：无
***************************************************************************/
void UsbCmdRun(void)
{
  UINT8 u8Cmd;
  UINT8 u8CheckSumState=CMD_NONE;

  while((Uart2GetRxDataLength() > 0) && (sUartCmdCb[USB_TYPE].eAssembleState != E_UART_CMD_RECEIVING_VALID))
  {
    Uart2CmdAssemble(Uart2GetByte());

    if(asTimer[TIMER_UART2].bEnable == TRUE)
    {
      asTimer[TIMER_UART2].bEnable = FALSE;
    }
  }
  
  //数据包有效，对可用的指令进行解析
    if(sUartCmdCb[USB_TYPE].eAssembleState == E_UART_CMD_RECEIVING_VALID)
    {
      u8Cmd = ConfigPackType(au8UsbBodyBuf, sUartCmdCb[USB_TYPE].u8BufDataLength);
      //建立索引表
      CreatCmdIndex(au8UsbBodyBuf);  
      
      switch(u8Cmd)
      {
        case XXPZ:
        {
          u8CheckSumState = ConfigInfoProcess(USB_TYPE, au8UsbBodyBuf);   //提取配置信息
          break;
        }
        
        case DMPZ:
        {
          u8CheckSumState = MainServerConfigProcess(USB_TYPE, au8UsbBodyBuf);   //提取配置信息
          break;
        }
        
        case S1PZ:
        {
          u8CheckSumState = Sub1ConfigProcess(USB_TYPE, au8UsbBodyBuf);   //提取配置信息
          break;
        }
        
        case S2PZ:
        {
          u8CheckSumState = Sub2ConfigProcess(USB_TYPE, au8UsbBodyBuf);   //提取配置信息
          break;
        }
        
        case S3PZ:
        {
          u8CheckSumState = Sub3ConfigProcess(USB_TYPE, au8UsbBodyBuf);   //提取配置信息
          break;
        }
        
        case S4PZ:
        {
          u8CheckSumState = Sub4ConfigProcess(USB_TYPE, au8UsbBodyBuf);   //提取配置信息
          break;
        }
        
        case ZDQD:
        {
          g_bTmlStart = TRUE;
          break;
        }
        
        case ZDTZ:
        {
          
          break;
        }
        
        case ZDGJ:
        {
          u8CheckSumState = PowerDownProcess(USB_TYPE, au8UsbBodyBuf);          
          break;
        }
        
        case TSKG:
        {
          u8CheckSumState = DebugProcess(USB_TYPE, au8UsbBodyBuf);
          break;
        }
        
        case BBHM:
        {
          u8CheckSumState = GetVersionProcess(USB_TYPE, au8UsbBodyBuf);
          break;
        }
        
        case YJCX:
        {
          u8CheckSumState = ReadDtuConfigProcess(USB_TYPE, au8UsbBodyBuf);
          break;
        }
        
        
        default: u8CheckSumState = CMD_NONE;
      }
      
//打印调试信息，配置状态 
    if(g_bDebug == TRUE)
    {
      switch(u8CheckSumState)
      {
        case CMD_NONE:  
        {
          UartSendString(UART2, "无效的配置命令！\r\n"); 
          break;
        }
        
        case CMD_XXPZ_OK:
        {
          UartSendString(UART2, "信息本地配置成功！\r\n");
          break;
        }
        
        case CMD_XXPZ_ERROR:
        {
          UartSendString(UART2, "信息本地配置失败！\r\n");
          break;
        }
        
        case CMD_DMPZ_OK:
        {
          UartSendString(UART2, "主服务器本地配置成功！\r\n");
          break;
        }
        
        case CMD_DMPZ_ERROR:
        {
          UartSendString(UART2, "主服务器本地配置失败！\r\n");
          break;
        }
        
        case CMD_S1PZ_OK:
        {
          UartSendString(UART2, "分中心1本地配置成功！\r\n");
          break;
        }
        
        case CMD_S1PZ_ERROR:
        {
          UartSendString(UART2, "分中心1本地配置失败！\r\n");
          break;
        }
        
        case CMD_S2PZ_OK:
        {
          UartSendString(UART2, "分中心2本地配置成功！\r\n");
          break;
        }
        
        case CMD_S2PZ_ERROR:
        {
          UartSendString(UART2, "分中心2本地配置失败！\r\n");
          break;
        }
        
        case CMD_S3PZ_OK:
        {
          UartSendString(UART2, "分中心3本地配置成功！\r\n");
          break;
        }
        
        case CMD_S3PZ_ERROR:
        {
          UartSendString(UART2, "分中心3本地配置失败！\r\n");
          break;
        }
        
        case CMD_S4PZ_OK:
        {
          UartSendString(UART2, "分中心4本地配置成功！\r\n");
          break;
        }
        
        case CMD_S4PZ_ERROR:
        {
          UartSendString(UART2, "分中心4本地配置失败！\r\n");
          break;
        }
        
        case CMD_TSKG_OK:
        {
          UartSendString(UART2, "调试开关配置成功！\r\n");
          break;
        }
        
        case CMD_TSKG_ERROR:
        {
          UartSendString(UART2, "调试开关配置失败！\r\n");
          break;
        }
        
        case CMD_BBHM_OK:
        {
          UartSendString(UART2, "固件版本读取成功！\r\n");
          break;
        }
        
        case CMD_BBHM_ERROR:
        {
          UartSendString(UART2, "固件版本读取失败！\r\n");
          break;
        }
        
        case CMD_DTHM_OK:
        {
          UartSendString(UART2, "DTU标识读取成功！\r\n");
          break;
        }
        
        case CMD_DTHM_ERROR:
        {
          UartSendString(UART2, "DTU标识读取失败！\r\n");
          break;
        }
        
        default: break;
      }
    }
        
      //处理完相关指令，清除buf
      MEM_SET(&au8UsbBodyBuf[0], 0, sizeof(au8UsbBodyBuf));
      sUartCmdCb[USB_TYPE].eAssembleState = E_UART_CMD_RECEIVING_HEADER;
      sUartCmdCb[USB_TYPE].u8BufDataLength = 0;
    }
    else
    {
      //规定时间内无数据，指令复位，重新提取有效数据
      if(asTimer[TIMER_UART2].bEnable == TRUE)                
      {
        if(asTimer[TIMER_UART2].u16LifeTime == 0)
        {
          sUartCmdCb[USB_TYPE].eAssembleState = E_UART_CMD_RECEIVING_HEADER;
          sUartCmdCb[USB_TYPE].u8BufDataLength = 0;

          asTimer[TIMER_UART2].bEnable = FALSE;
        }	
       }
       else
       {
          if(sUartCmdCb[USB_TYPE].u8BufDataLength != 0)	
          {
            asTimer[TIMER_UART2].u16LifeTime = asTimer[TIMER_UART2].u16LifeOldTime;
            asTimer[TIMER_UART2].bEnable = TRUE;
          }
        }	
     }
}



/***************************************************************************
//函数名称：UartCmdInit
//功能描述：串口接收命令解析
//
//参数：无
//返回：无
***************************************************************************/
void UartCmdInit(void)
{
  /* gps命令解析初始化 */
  GpsCmdInit();
  
  /* DTU命令解析初始化 */
  DtuCmdInit();
  
  /* USB转串口命令解析初始化 */
  UsbCmdInit();
  
  /* 无线模块命令解析初始化 */
//  WirelessCmdInit();
}

/***************************************************************************
//函数名称：UartCmdRun
//功能描述：串口接收命令解析
//
//参数：无
//返回：无
***************************************************************************/
void UartCmdRun(void)
{
  /* gps命令解析 */
  GpsCmdRun();
  
  /* DTU命令解析 */
  DtuCmdRun();
  
  /* usb转串口命令解析 */
  UsbCmdRun();
  
  /* 无线传输命令解析 */
//  WirelessCmdRun();       //暂时没有使用，不做处理
  
  /* DTU信息发送 */
  DtuInfoSend();          //讲采集的数据以数据包ASCII码形式发送给中心
}