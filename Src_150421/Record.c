/******************************************************************************
*											
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved										
*
* 文 件 名： Record.C
* 描    述： 记录

* 创建日期： 2014年9月5日11:31:59
* 作    者： Bob
* 当前版本： V1.00

------------------------------------------------------
*******************************************************************************/
#include "msp430x54x.h"

#include "DataType.h"
#include "global.h"
#include "string.h"
#include "ctype.h"
#include "I2C_Driver.h"
#include "Record.h"
#include "SystemCtl.h"


/********************************************************************************
 * 函数名称：RecordInit
 * 函数功能：初始化记录信息，包括设备编号，设备信息等
 *           
 * EEPROM内部存储结构对应表： 地址       数据描述
                              0x00       0xAA -- 开机标志
                                         0x55 -- 关机标志
                              ---------------------------
                              0x01       0~99 -- 设备编号
                              ---------------------------
                              0x02        信息长度
                              0x03
                               ~          设备信息
                              0x22
                              ---------------------------
                              0x23  
                               ~          上次关机时间戳
                              0x26
                              ---------------------------
                              0x27        
                               ~          关机时记录日期
                              0x30  
 *                               
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/

//检查正常开关机状态
BOOL CheckPowerDown(void)
{
   //读取系统的开关机状态，判断是正常/非正常关机
  g_u8SystemState = I2CMasterReadOneByte(EEPROM_READ_ADDR, SYSTEM_STATE_ADDR);
  
  if(g_u8SystemState == 0x55)
  {
    return TRUE;
  }
  else
  {
    if(g_u8SystemState == 0xAA)
    {
      return FALSE;
    }
  }
  
  return FALSE;
}

/********************************************************************************
 * 函数名称：ReadDtuMainServer
 * 函数功能：读取DTU模块备份配置信息。
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void ReadDtuMainServer(void)
{
  UINT8 i=0;
  UINT8 au8Temp[12];
  MEM_SET(&sMainServer.bCfgTcpUdp, 0, sizeof(_tsMainServer));
  
  //读取分中心的备份信息到配置结构
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_MAIN_COM_ADDR, &sMainServer.au8BckComIndex[0], 2);
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_MAIN_TCP_ADDR, &sMainServer.bBckTcpUdp, 1);
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_MAIN_APN_ADDR, &sMainServer.bBckApn, 1);
  
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_MAIN_NAME_LEN, &sMainServer.u8BckUserNameLen, 1);
  if(sMainServer.u8BckUserNameLen > 16) sMainServer.u8BckUserNameLen = 16;
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_MAIN_NAME_ADDR, &sMainServer.au8BckUserName[0], sMainServer.u8BckUserNameLen);
  
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_MAIN_PASS_LEN, &sMainServer.u8BckPasswordLen, 1);
  if(sMainServer.u8BckPasswordLen > 16) sMainServer.u8BckPasswordLen = 16;
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_MAIN_PASS_ADDR, &sMainServer.au8BckPassword[0], sMainServer.u8BckPasswordLen);
  
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_MAIN_LEN_ADDR, &sMainServer.u8BckIpLen, 1);
  if(sMainServer.u8BckIpLen > 64) sMainServer.u8BckIpLen = 64;
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_MAIN_IP_ADDR, &sMainServer.au8BckIpAddr[0], sMainServer.u8BckIpLen);
  
  //从存储器中读取设备信息, 中心分配给设备的编号赋给DTU的ID号
  for(i=0; i<11; i++)
  {
    au8Temp[i] = I2CMasterReadOneByte(EEPROM_READ_ADDR, (DTU_INDEX_ADDR+i));
  }
  
  MEM_CPY(&sMainServer.au8BckId[0], &au8Temp[0], 11);
  MEM_SET(&au8Temp[0], 0, 11);
  GetDtuConfigInfo(&au8Temp[0]);
  
  if(0 != MEM_CMP(&sMainServer.au8BckId[0], &au8Temp[0], 11))
  {
   I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_INDEX_ADDR, &au8Temp[0], 11);
   MEM_CPY(&sMainServer.au8BckId[0], &au8Temp[0], 11);
  }
}

/********************************************************************************
 * 函数名称：WriteDtuMainServer
 * 函数功能：将配置信息写入存储器。
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void WriteDtuMainServer(void)
{
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_MAIN_COM_ADDR, &sMainServer.au8BckComIndex[0], 2);
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_MAIN_TCP_ADDR, &sMainServer.bBckTcpUdp, 1);
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_MAIN_APN_ADDR, &sMainServer.bBckApn, 1);
  
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_MAIN_NAME_LEN, &sMainServer.u8BckUserNameLen, 1);
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_MAIN_NAME_ADDR, &sMainServer.au8BckUserName[0], sMainServer.u8BckUserNameLen);
  
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_MAIN_PASS_LEN,  &sMainServer.u8BckPasswordLen, 1);
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_MAIN_PASS_ADDR, &sMainServer.au8BckPassword[0], sMainServer.u8BckPasswordLen);
  
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_MAIN_LEN_ADDR, &sMainServer.u8BckIpLen, 1);
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_MAIN_IP_ADDR, &sMainServer.au8BckIpAddr[0], sMainServer.u8BckIpLen);
  
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_INDEX_ADDR, &sMainServer.au8BckId[0], 11);
}

/********************************************************************************
 * 函数名称：ReadDtuMainServer
 * 函数功能：读取DTU模块分中心1配置。
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void ReadDtuSub1Config(void)
{
  // 初始化DTU配置结构
  MEM_SET(&sDtuCfg[IP_SUB_1].au8CfgIpAddr[0], 0, sizeof(_tsDtuCfg));
  //DTU分中心1配置信息备份
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB1_COM_ADDR, &sDtuCfg[IP_SUB_1].au8BckComIndex[0], 2);
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB1_EN_ADDR, &sDtuCfg[IP_SUB_1].bBckEnable, 1);
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB1_LEN_ADDR, &sDtuCfg[IP_SUB_1].u8BckIpLen, 1);
  if(sDtuCfg[IP_SUB_1].u8BckIpLen > 61) sDtuCfg[IP_SUB_1].u8BckIpLen = 61;
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB1_IP_ADDR,  &sDtuCfg[IP_SUB_1].au8BckIpAddr[0], sDtuCfg[IP_SUB_1].u8BckIpLen);
}

/********************************************************************************
 * 函数名称：ReadDtuMainServer
 * 函数功能：配置DTU分中心2。
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void WriteDtuSub1Config(void)
{
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_SUB1_COM_ADDR, &sDtuCfg[IP_SUB_1].au8BckComIndex[0], 2);
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_SUB1_EN_ADDR, &sDtuCfg[IP_SUB_1].bBckEnable, 1);
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_SUB1_LEN_ADDR, &sDtuCfg[IP_SUB_1].u8BckIpLen, 1);
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_SUB1_IP_ADDR,  &sDtuCfg[IP_SUB_1].au8BckIpAddr[0], sDtuCfg[IP_SUB_1].u8BckIpLen);
}

/********************************************************************************
 * 函数名称：ReadDtuSub2Config
 * 函数功能：读取DTU模块分中心2配置。
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void ReadDtuSub2Config(void)
{
  // 初始化DTU配置结构
  MEM_SET(&sDtuCfg[IP_SUB_2].au8CfgIpAddr[0], 0, sizeof(_tsDtuCfg));
  //DTU分中心1配置信息备份
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB2_COM_ADDR, &sDtuCfg[IP_SUB_2].au8BckComIndex[0], 2);
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB2_EN_ADDR, &sDtuCfg[IP_SUB_2].bBckEnable, 1);
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB2_LEN_ADDR, &sDtuCfg[IP_SUB_2].u8BckIpLen, 1);
  if(sDtuCfg[IP_SUB_2].u8BckIpLen > 61) sDtuCfg[IP_SUB_2].u8BckIpLen = 61;
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB2_IP_ADDR,  &sDtuCfg[IP_SUB_2].au8BckIpAddr[0], sDtuCfg[IP_SUB_2].u8BckIpLen);
}

/********************************************************************************
 * 函数名称：WriteDtuSub2Config
 * 函数功能：配置DTU分中心2
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void WriteDtuSub2Config(void)
{
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_SUB2_COM_ADDR, &sDtuCfg[IP_SUB_2].au8BckComIndex[0], 2);
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_SUB2_EN_ADDR, &sDtuCfg[IP_SUB_2].bBckEnable, 1);
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_SUB2_LEN_ADDR, &sDtuCfg[IP_SUB_2].u8BckIpLen, 1);
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_SUB2_IP_ADDR,  &sDtuCfg[IP_SUB_2].au8BckIpAddr[0], sDtuCfg[IP_SUB_2].u8BckIpLen);
}

/********************************************************************************
 * 函数名称：ReadDtuSub3Config
 * 函数功能：读取DTU模块分中心3配置。
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void ReadDtuSub3Config(void)
{
  // 初始化DTU配置结构
  MEM_SET(&sDtuCfg[IP_SUB_3].au8CfgIpAddr[0], 0, sizeof(_tsDtuCfg));
  //DTU分中心1配置信息备份
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB3_COM_ADDR, &sDtuCfg[IP_SUB_3].au8BckComIndex[0], 2);
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB3_EN_ADDR, &sDtuCfg[IP_SUB_3].bBckEnable, 1);
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB3_LEN_ADDR, &sDtuCfg[IP_SUB_3].u8BckIpLen, 1);
  if(sDtuCfg[IP_SUB_3].u8BckIpLen > 61) sDtuCfg[IP_SUB_3].u8BckIpLen = 61;
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB3_IP_ADDR,  &sDtuCfg[IP_SUB_3].au8BckIpAddr[0], sDtuCfg[IP_SUB_3].u8BckIpLen);
}

/********************************************************************************
 * 函数名称：WriteDtuSub3Config
 * 函数功能：配置DTU分中心3
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void WriteDtuSub3Config(void)
{
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_SUB3_COM_ADDR, &sDtuCfg[IP_SUB_3].au8BckComIndex[0], 2);
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_SUB3_EN_ADDR, &sDtuCfg[IP_SUB_3].bBckEnable, 1);
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_SUB3_LEN_ADDR, &sDtuCfg[IP_SUB_3].u8BckIpLen, 1);
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_SUB3_IP_ADDR,  &sDtuCfg[IP_SUB_3].au8BckIpAddr[0], sDtuCfg[IP_SUB_3].u8BckIpLen);
}

/********************************************************************************
 * 函数名称：ReadDtuSub4Config
 * 函数功能：读取DTU模块分中心4配置。
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void ReadDtuSub4Config(void)
{
  // 初始化DTU配置结构
  MEM_SET(&sDtuCfg[IP_SUB_4].au8CfgIpAddr[0], 0, sizeof(_tsDtuCfg));
  //DTU分中心1配置信息备份
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB4_COM_ADDR, &sDtuCfg[IP_SUB_4].au8BckComIndex[0], 2);
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB4_EN_ADDR, &sDtuCfg[IP_SUB_4].bBckEnable, 1);
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB4_LEN_ADDR, &sDtuCfg[IP_SUB_4].u8BckIpLen, 1);
  if(sDtuCfg[IP_SUB_4].u8BckIpLen > 61) sDtuCfg[IP_SUB_4].u8BckIpLen = 61;
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB4_IP_ADDR,  &sDtuCfg[IP_SUB_4].au8BckIpAddr[0], sDtuCfg[IP_SUB_4].u8BckIpLen);
}

/********************************************************************************
 * 函数名称：WriteDtuSub4Config
 * 函数功能：配置DTU分中心4
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void WriteDtuSub4Config(void)
{
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_SUB4_COM_ADDR, &sDtuCfg[IP_SUB_4].au8BckComIndex[0], 2);
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_SUB4_EN_ADDR, &sDtuCfg[IP_SUB_4].bBckEnable, 1);
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_SUB4_LEN_ADDR, &sDtuCfg[IP_SUB_4].u8BckIpLen, 1);
  I2CMasterWrite(EEPROM_WRITE_ADDR, DTU_SUB4_IP_ADDR,  &sDtuCfg[IP_SUB_4].au8BckIpAddr[0], sDtuCfg[IP_SUB_4].u8BckIpLen);
}

/********************************************************************************
 * 函数名称：WriteDisplayConfig
 * 函数功能：存储显示信息
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void WriteDisplayConfig(void)
{
  //存储当前设备编号
  I2CMasterWriteOneByte(EEPROM_WRITE_ADDR, (TERMINAL_INDEX_ADDR),   (UINT8)((sDisplay.u32TmlIndex & 0xff000000) >> 24) );
  I2CMasterWriteOneByte(EEPROM_WRITE_ADDR, (TERMINAL_INDEX_ADDR+1), (UINT8)((sDisplay.u32TmlIndex & 0x00ff0000) >> 16) );
  I2CMasterWriteOneByte(EEPROM_WRITE_ADDR, (TERMINAL_INDEX_ADDR+2), (UINT8)((sDisplay.u32TmlIndex & 0x0000ff00) >> 8) );
  I2CMasterWriteOneByte(EEPROM_WRITE_ADDR, (TERMINAL_INDEX_ADDR+3), (UINT8)(sDisplay.u32TmlIndex & 0x000000ff) );
  
  //存储当前设备类型
  I2CMasterWriteOneByte(EEPROM_WRITE_ADDR, TERMINAL_TYPE_ADDR, sDisplay.u8TmlType);
  
  //存储当前设备信息
  I2CMasterWrite(EEPROM_READ_ADDR, TERMINAL_INFO_ADDR, &sDisplay.au8TmlInfo[0], sDisplay.u8InfoLen);
      
  //读取信息长度和设备型号，把剩余空间赋值为空格
  I2CMasterWriteOneByte(EEPROM_READ_ADDR, TERMINAL_INFO_LEN, sDisplay.u8InfoLen);
      
  //存储当前发送时间间隔
  I2CMasterWriteOneByte(EEPROM_WRITE_ADDR, TIME_SPACE_ADDR, sDisplay.u8TimeSpace);
}

/********************************************************************************
 * 函数名称：ReadDebugSwtich
 * 函数功能：读调试开关
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void ReadDebugSwtich(void)
{
   g_bDebug = I2CMasterReadOneByte(EEPROM_WRITE_ADDR, DEBUG_EN_ADDR);
}

/********************************************************************************
 * 函数名称：WriteDebugSwtich
 * 函数功能：写调试信息接口
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void WriteDebugSwtich(void)
{
   I2CMasterWriteOneByte(EEPROM_WRITE_ADDR, DEBUG_EN_ADDR, g_bDebug);
}


/********************************************************************************
 * 函数名称：RecordInit
 * 函数功能：信息初始化
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void RecordInit(void)
{
  //读取调试信息接口
  ReadDebugSwtich();
  
  //读取DTU主服务器配置
  ReadDtuMainServer();
  
  //读取DTU分中心1配置
  ReadDtuSub1Config();
  
  //读取DTU分中心2配置
  ReadDtuSub2Config();
  
  //读取DTU分中心3配置
  ReadDtuSub3Config();
  
  //读取DTU分中心4配置
  ReadDtuSub4Config();

}

/********************************************************************************
 * 函数名称：RecordRun
 * 函数功能：接收串口发送的配置数据，并显示在显示器上
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
void RecordRun(void)
{
  if((g_u8NeedSaved & SAVE_ALL_CFG) != SAVE_NONE)
  {
  
    //需要保存设备信息
    if(g_u8NeedSaved & SAVE_TML_INFO)
    {
      WriteDisplayConfig();
    }
    
    //需要保存调试信息接口使能
    if(g_u8NeedSaved & SAVE_DEBUG_SW)
    { 
      WriteDebugSwtich();
    }
    
    //需要保存DTU配置信息
    if(g_u8NeedSaved & SAVE_DTU_CFG)
    {
      switch(g_u8NeedSaved & SAVE_DTU_CFG)
      {
        case SAVE_DTU_MAIN:
        {
          WriteDtuMainServer();          
          break;
        }
        
        case SAVE_DTU_SUB1:
        {
          WriteDtuSub1Config();          
          break;
        }
        
        case SAVE_DTU_SUB2:
        {
          WriteDtuSub2Config();          
          break;
        }
        
        case SAVE_DTU_SUB3:
        {
          WriteDtuSub3Config();          
          break;
        }
        
        case SAVE_DTU_SUB4:
        {
          WriteDtuSub4Config();  
          break;
        }
        
        default: break;
      }
    }
      
    g_u8NeedSaved = SAVE_NONE;
  }
}