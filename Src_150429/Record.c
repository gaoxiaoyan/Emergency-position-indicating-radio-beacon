/******************************************************************************
*											
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved										
*
* �� �� ���� Record.C
* ��    ���� ��¼

* �������ڣ� 2014��9��5��11:31:59
* ��    �ߣ� Bob
* ��ǰ�汾�� V1.00

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
 * �������ƣ�RecordInit
 * �������ܣ���ʼ����¼��Ϣ�������豸��ţ��豸��Ϣ��
 *           
 * EEPROM�ڲ��洢�ṹ��Ӧ�� ��ַ       ��������
                              0x00       0xAA -- ������־
                                         0x55 -- �ػ���־
                              ---------------------------
                              0x01       0~99 -- �豸���
                              ---------------------------
                              0x02        ��Ϣ����
                              0x03
                               ~          �豸��Ϣ
                              0x22
                              ---------------------------
                              0x23  
                               ~          �ϴιػ�ʱ���
                              0x26
                              ---------------------------
                              0x27        
                               ~          �ػ�ʱ��¼����
                              0x30  
 *                               
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/

//����������ػ�״̬
BOOL CheckPowerDown(void)
{
   //��ȡϵͳ�Ŀ��ػ�״̬���ж�������/�������ػ�
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
 * �������ƣ�ReadDtuMainServer
 * �������ܣ���ȡDTUģ�鱸��������Ϣ��
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void ReadDtuMainServer(void)
{
  UINT8 i=0;
  UINT8 au8Temp[12];
  MEM_SET(&sMainServer.bCfgTcpUdp, 0, sizeof(_tsMainServer));
  
  //��ȡ�����ĵı�����Ϣ�����ýṹ
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
  
  //�Ӵ洢���ж�ȡ�豸��Ϣ, ���ķ�����豸�ı�Ÿ���DTU��ID��
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
 * �������ƣ�WriteDtuMainServer
 * �������ܣ���������Ϣд��洢����
 *
 * ���룺 ��
 * ���أ� ��
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
 * �������ƣ�ReadDtuMainServer
 * �������ܣ���ȡDTUģ�������1���á�
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void ReadDtuSub1Config(void)
{
  // ��ʼ��DTU���ýṹ
  MEM_SET(&sDtuCfg[IP_SUB_1].au8CfgIpAddr[0], 0, sizeof(_tsDtuCfg));
  //DTU������1������Ϣ����
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB1_COM_ADDR, &sDtuCfg[IP_SUB_1].au8BckComIndex[0], 2);
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB1_EN_ADDR, &sDtuCfg[IP_SUB_1].bBckEnable, 1);
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB1_LEN_ADDR, &sDtuCfg[IP_SUB_1].u8BckIpLen, 1);
  if(sDtuCfg[IP_SUB_1].u8BckIpLen > 61) sDtuCfg[IP_SUB_1].u8BckIpLen = 61;
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB1_IP_ADDR,  &sDtuCfg[IP_SUB_1].au8BckIpAddr[0], sDtuCfg[IP_SUB_1].u8BckIpLen);
}

/********************************************************************************
 * �������ƣ�ReadDtuMainServer
 * �������ܣ�����DTU������2��
 *
 * ���룺 ��
 * ���أ� ��
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
 * �������ƣ�ReadDtuSub2Config
 * �������ܣ���ȡDTUģ�������2���á�
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void ReadDtuSub2Config(void)
{
  // ��ʼ��DTU���ýṹ
  MEM_SET(&sDtuCfg[IP_SUB_2].au8CfgIpAddr[0], 0, sizeof(_tsDtuCfg));
  //DTU������1������Ϣ����
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB2_COM_ADDR, &sDtuCfg[IP_SUB_2].au8BckComIndex[0], 2);
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB2_EN_ADDR, &sDtuCfg[IP_SUB_2].bBckEnable, 1);
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB2_LEN_ADDR, &sDtuCfg[IP_SUB_2].u8BckIpLen, 1);
  if(sDtuCfg[IP_SUB_2].u8BckIpLen > 61) sDtuCfg[IP_SUB_2].u8BckIpLen = 61;
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB2_IP_ADDR,  &sDtuCfg[IP_SUB_2].au8BckIpAddr[0], sDtuCfg[IP_SUB_2].u8BckIpLen);
}

/********************************************************************************
 * �������ƣ�WriteDtuSub2Config
 * �������ܣ�����DTU������2
 *
 * ���룺 ��
 * ���أ� ��
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
 * �������ƣ�ReadDtuSub3Config
 * �������ܣ���ȡDTUģ�������3���á�
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void ReadDtuSub3Config(void)
{
  // ��ʼ��DTU���ýṹ
  MEM_SET(&sDtuCfg[IP_SUB_3].au8CfgIpAddr[0], 0, sizeof(_tsDtuCfg));
  //DTU������1������Ϣ����
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB3_COM_ADDR, &sDtuCfg[IP_SUB_3].au8BckComIndex[0], 2);
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB3_EN_ADDR, &sDtuCfg[IP_SUB_3].bBckEnable, 1);
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB3_LEN_ADDR, &sDtuCfg[IP_SUB_3].u8BckIpLen, 1);
  if(sDtuCfg[IP_SUB_3].u8BckIpLen > 61) sDtuCfg[IP_SUB_3].u8BckIpLen = 61;
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB3_IP_ADDR,  &sDtuCfg[IP_SUB_3].au8BckIpAddr[0], sDtuCfg[IP_SUB_3].u8BckIpLen);
}

/********************************************************************************
 * �������ƣ�WriteDtuSub3Config
 * �������ܣ�����DTU������3
 *
 * ���룺 ��
 * ���أ� ��
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
 * �������ƣ�ReadDtuSub4Config
 * �������ܣ���ȡDTUģ�������4���á�
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void ReadDtuSub4Config(void)
{
  // ��ʼ��DTU���ýṹ
  MEM_SET(&sDtuCfg[IP_SUB_4].au8CfgIpAddr[0], 0, sizeof(_tsDtuCfg));
  //DTU������1������Ϣ����
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB4_COM_ADDR, &sDtuCfg[IP_SUB_4].au8BckComIndex[0], 2);
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB4_EN_ADDR, &sDtuCfg[IP_SUB_4].bBckEnable, 1);
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB4_LEN_ADDR, &sDtuCfg[IP_SUB_4].u8BckIpLen, 1);
  if(sDtuCfg[IP_SUB_4].u8BckIpLen > 61) sDtuCfg[IP_SUB_4].u8BckIpLen = 61;
  I2CMasterRead(EEPROM_WRITE_ADDR, DTU_SUB4_IP_ADDR,  &sDtuCfg[IP_SUB_4].au8BckIpAddr[0], sDtuCfg[IP_SUB_4].u8BckIpLen);
}

/********************************************************************************
 * �������ƣ�WriteDtuSub4Config
 * �������ܣ�����DTU������4
 *
 * ���룺 ��
 * ���أ� ��
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
 * �������ƣ�WriteDisplayConfig
 * �������ܣ��洢��ʾ��Ϣ
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void WriteDisplayConfig(void)
{
  //�洢��ǰ�豸���
  I2CMasterWriteOneByte(EEPROM_WRITE_ADDR, (TERMINAL_INDEX_ADDR),   (UINT8)((sDisplay.u32TmlIndex & 0xff000000) >> 24) );
  I2CMasterWriteOneByte(EEPROM_WRITE_ADDR, (TERMINAL_INDEX_ADDR+1), (UINT8)((sDisplay.u32TmlIndex & 0x00ff0000) >> 16) );
  I2CMasterWriteOneByte(EEPROM_WRITE_ADDR, (TERMINAL_INDEX_ADDR+2), (UINT8)((sDisplay.u32TmlIndex & 0x0000ff00) >> 8) );
  I2CMasterWriteOneByte(EEPROM_WRITE_ADDR, (TERMINAL_INDEX_ADDR+3), (UINT8)(sDisplay.u32TmlIndex & 0x000000ff) );
  
  //�洢��ǰ�豸����
  I2CMasterWriteOneByte(EEPROM_WRITE_ADDR, TERMINAL_TYPE_ADDR, sDisplay.u8TmlType);
  
  //�洢��ǰ�豸��Ϣ
  I2CMasterWrite(EEPROM_READ_ADDR, TERMINAL_INFO_ADDR, &sDisplay.au8TmlInfo[0], sDisplay.u8InfoLen);
      
  //��ȡ��Ϣ���Ⱥ��豸�ͺţ���ʣ��ռ丳ֵΪ�ո�
  I2CMasterWriteOneByte(EEPROM_READ_ADDR, TERMINAL_INFO_LEN, sDisplay.u8InfoLen);
      
  //�洢��ǰ����ʱ����
  I2CMasterWriteOneByte(EEPROM_WRITE_ADDR, TIME_SPACE_ADDR, sDisplay.u8TimeSpace);
}

/********************************************************************************
 * �������ƣ�ReadDebugSwtich
 * �������ܣ������Կ���
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void ReadDebugSwtich(void)
{
   g_bDebug = I2CMasterReadOneByte(EEPROM_WRITE_ADDR, DEBUG_EN_ADDR);
}

/********************************************************************************
 * �������ƣ�WriteDebugSwtich
 * �������ܣ�д������Ϣ�ӿ�
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void WriteDebugSwtich(void)
{
   I2CMasterWriteOneByte(EEPROM_WRITE_ADDR, DEBUG_EN_ADDR, g_bDebug);
}


/********************************************************************************
 * �������ƣ�RecordInit
 * �������ܣ���Ϣ��ʼ��
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void RecordInit(void)
{
  //��ȡ������Ϣ�ӿ�
  ReadDebugSwtich();
  
  //��ȡDTU������������
  ReadDtuMainServer();
  
  //��ȡDTU������1����
  ReadDtuSub1Config();
  
  //��ȡDTU������2����
  ReadDtuSub2Config();
  
  //��ȡDTU������3����
  ReadDtuSub3Config();
  
  //��ȡDTU������4����
  ReadDtuSub4Config();

}

/********************************************************************************
 * �������ƣ�RecordRun
 * �������ܣ����մ��ڷ��͵��������ݣ�����ʾ����ʾ����
 *
 * ���룺 ��
 * ���أ� ��
 *
 *******************************************************************************/
void RecordRun(void)
{
  if((g_u8NeedSaved & SAVE_ALL_CFG) != SAVE_NONE)
  {
  
    //��Ҫ�����豸��Ϣ
    if(g_u8NeedSaved & SAVE_TML_INFO)
    {
      WriteDisplayConfig();
    }
    
    //��Ҫ���������Ϣ�ӿ�ʹ��
    if(g_u8NeedSaved & SAVE_DEBUG_SW)
    { 
      WriteDebugSwtich();
    }
    
    //��Ҫ����DTU������Ϣ
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