
/*************************************************************************************************************************
 *										 Copyright (c) 2014 Boy.Yang  All rights reserved
 *
 * �ļ�����I2C_Driver.C
 * 
 * ��  �ߣ�BOB.YANG
 *
 * ��  ����I2C���ߵײ���������
 *
 * ��Ҫ������
 *
 * ======================================================================================================================
 * �汾��¼��
 * 1��V1.0 	2014��3��10��22:02:23
 *
 *************************************************************************************************************************/
#include "msp430x54x.h"

#include "DataType.h"
#include "I2C_Driver.h"
#include "global.h"

#define  I2CSDA_SET_1       I2cSdaOut |=  I2cSda
#define  I2CSDA_SET_0       I2cSdaOut &=~ I2cSda

#define  I2CSCL_SET_1       I2cSclOut |=  I2cScl
#define  I2CSCL_SET_0       I2cSclOut &=~ I2cScl

#define  I2CSDA_INPUT_IN    I2cSdaIn & I2cSda

void delay5us( void )
{
  unsigned char count=32;
  while(count--);
}     

void Delay_MS( unsigned int m )
{    
  unsigned int i,j;
  
  for(i=0;i<m;i++)
    for(j=0;j<50;j++);
} 

//I2C�ӿڷ������
void I2C_Pins_DIR_Setting ( unsigned char SDADIR )
{
  I2cSclDir |= I2cScl;
      
  if(SDADIR == 0)
  {
    I2cSdaDir  &= ~I2cSda;
    I2cSdaPort &= ~I2cSda;
  }
  else
  {
    if(SDADIR == 1)
    {
      I2cSdaDir |= I2cSda;  
    }
  }
}

void Engender_I2C_start_signal(void)
{
  I2CSCL_SET_1;  delay5us();
  I2CSDA_SET_1;  delay5us();
  I2CSDA_SET_0;  delay5us();
}       
void Engender_I2C_stop_signal(void)
{ 
  I2CSDA_SET_0;  delay5us();
  I2CSCL_SET_1;  delay5us(); 
  I2CSDA_SET_1;  delay5us();
}       

void Engender_I2C_ack_signal(void)
{     
  I2CSCL_SET_0;  delay5us();
  I2CSDA_SET_0;  delay5us();
  I2CSCL_SET_1;  delay5us();
  I2CSCL_SET_0;  delay5us();
}          
void Engender_I2C_noack_signal(void)   
{ 
  I2CSCL_SET_0;  delay5us();
  I2CSDA_SET_1;  delay5us();
  I2CSCL_SET_1;  delay5us();
  I2CSCL_SET_0;  delay5us();
}  

void I2C_WRITE_BYTE( unsigned char BytEDAta )   
{ 
  unsigned char count;
  for(count=0;count<8;count++)
  {
    I2CSCL_SET_0;
    delay5us();
    if(BytEDAta&0x80)I2CSDA_SET_1;
    else             I2CSDA_SET_0;
    delay5us();
    I2CSCL_SET_1;
    delay5us();
    BytEDAta<<=1;
  }
}          

UINT8 I2C_READ_BYTE( void )
{ 
  unsigned char count,readbyte=0; 
  I2CSCL_SET_0; 
  for(count=0;count<8;count++)
  {
    readbyte=readbyte<<1; 
    I2CSCL_SET_1; 
    delay5us();  
    if(I2CSDA_INPUT_IN)readbyte=readbyte|0x01;
    else               readbyte=readbyte&0xfe;
    delay5us();
    I2CSCL_SET_0;
    delay5us(); 
  }
  I2CSCL_SET_0;
  return readbyte;
}        

void I2CMasterWriteOneByte(UINT8 u8DeviceAddr, UINT16 u16Address, UINT8 u8Date)
{
  UINT8 u8PageSel;
  UINT8 u8Address;
  
  u8Address = (UINT8)(u16Address  & 0xff);
  
  if(u16Address > 0xff)
  {
    u8PageSel = BIT1; //pageѡ����������ַ��bit1
  }
  else
  {
    u8PageSel = 0;
  }
  I2C_Pins_DIR_Setting(1);
  Engender_I2C_start_signal();
  I2C_WRITE_BYTE((u8DeviceAddr & 0xfe) | u8PageSel);
  Engender_I2C_ack_signal();
  I2C_WRITE_BYTE(u8Address);
  Engender_I2C_ack_signal();
  I2C_WRITE_BYTE(u8Date);
  Engender_I2C_ack_signal();
  Engender_I2C_stop_signal();
  I2CSCL_SET_1;
  I2CSDA_SET_1;
  Delay_MS(1);
}      

UINT8 I2CMasterReadOneByte(UINT8 u8DeviceAddr, UINT16 u16Address)
{ 
  UINT8 readdate;
  UINT8 u8PageSel;
  UINT8 u8Address;
  
  u8Address = (UINT8)(u16Address  & 0xff);
  
  if(u16Address > 0xff)
  {
    u8PageSel = BIT1; //pageѡ����������ַ��bit1
  }
  else
  {
    u8PageSel = 0;
  }

  I2C_Pins_DIR_Setting(1);
  Engender_I2C_start_signal();
  I2C_WRITE_BYTE((u8DeviceAddr & 0xfe) | u8PageSel);
  Engender_I2C_ack_signal();
  I2C_WRITE_BYTE(u8Address); 
  Engender_I2C_ack_signal();
//  Delay_MS(10);         
  Engender_I2C_start_signal();
  I2C_WRITE_BYTE(u8DeviceAddr | 0x01 | u8PageSel);  
  I2C_Pins_DIR_Setting(0);
  Engender_I2C_ack_signal();
  readdate=I2C_READ_BYTE();
  I2C_Pins_DIR_Setting(1);     
  Engender_I2C_noack_signal();
  Engender_I2C_stop_signal();
  I2CSCL_SET_1;
  I2CSDA_SET_1;    
  Delay_MS(1);
  return (readdate);
}

/*************************************************************************************************************
 * �������ƣ� I2CMasterWrite
 * ���������� I2C���� д�ӻ�����
 * ��  �룺 UINT8 u8Add, �ӻ���ַ
 *          UINT8 *Buffer, 	
 *          UINT8 ToWrite	дbuffer�ĸ���
 * ��  ����	NO_VALUABLE_DATA / I2C_OK 
 * ȫ�ֱ�����
 * ����ģ�飺
 ************************************************************************************************************/
 void I2CMasterWrite(UINT8 u8DeviceAddr, UINT16 u16Addr, UINT8 *Buffer, UINT8 ToWrite)
 { /*
   UINT8 u8PageSel,i;
  UINT8 u8Address;
  
  u8Address = (UINT8)(u16Addr  & 0xff);
  
  if(u16Addr > 0xff)
  {
    u8PageSel = BIT1; //pageѡ����������ַ��bit1
  }
  else
  {
    u8PageSel = 0;
  }
  I2C_Pins_DIR_Setting(1);
  Engender_I2C_start_signal();
  I2C_WRITE_BYTE((u8DeviceAddr & 0xfe) | u8PageSel);
  Engender_I2C_ack_signal();
  I2C_WRITE_BYTE(u8Address);
  Engender_I2C_ack_signal();
  
  for(i=ToWrite; i>0; i--)
  {
    I2C_WRITE_BYTE(*Buffer);
    Engender_I2C_ack_signal();
    Buffer++;
  }
  Engender_I2C_stop_signal();
  I2CSCL_SET_1;
  I2CSDA_SET_1;
  Delay_MS(1);
 */
    UINT16 u16TempAddr;
    UINT16 i;
    
    u16TempAddr = u16Addr;
    
    for(i=ToWrite; i>0; i--)
    {
      I2CMasterWriteOneByte(u8DeviceAddr, u16TempAddr, *Buffer);
      
      u16TempAddr++;
      Buffer++;
    }
 }
   
/*************************************************************************************************************
 * �������ƣ� I2CMasterRead
 * ���������� I2C���� д�ӻ�����
 * ��  �룺 UINT8 SlaveAdd, �ӻ���ַ
 *          UINT8 *Buffer, 	
 *          UINT8 ToRead	��buffer�ĸ���
 * ��  ����	NO_VALUABLE_DATA / I2C_OK 						
 * ȫ�ֱ�����
 * ����ģ�飺
 ************************************************************************************************************/
 void I2CMasterRead(UINT8 u8DeviceAddr, UINT16 u16Addr, UINT8 *Buffer, UINT8 ToRead)
 {/*
  UINT8 readdate, i;
  UINT8 u8PageSel;
  UINT8 u8Address;
  
  u8Address = (UINT8)(u16Addr  & 0xff);
  
  if(u16Addr > 0xff)
  {
    u8PageSel = BIT1; //pageѡ����������ַ��bit1
  }
  else
  {
    u8PageSel = 0;
  }

  I2C_Pins_DIR_Setting(1);
  Engender_I2C_start_signal();
  I2C_WRITE_BYTE((u8DeviceAddr & 0xfe) | u8PageSel);
  Engender_I2C_ack_signal();
  I2C_WRITE_BYTE(u8Address); 
  Engender_I2C_ack_signal();
//  Delay_MS(10);         
  Engender_I2C_start_signal();
  I2C_WRITE_BYTE(u8DeviceAddr | 0x01 | u8PageSel);  
  Engender_I2C_ack_signal();
  
  for(i=ToRead; i>1; i--)
  {
    I2C_Pins_DIR_Setting(0);
    *Buffer++ = I2C_READ_BYTE();
    
    I2C_Pins_DIR_Setting(1);     
    Engender_I2C_ack_signal();
  }
  
  I2C_Pins_DIR_Setting(0);
  *Buffer++ = I2C_READ_BYTE();
  I2C_Pins_DIR_Setting(1); 
  Engender_I2C_noack_signal();
  Engender_I2C_stop_signal();
  I2CSCL_SET_1;
  I2CSDA_SET_1;    
  Delay_MS(1);
 */
    UINT16 u16TempAddr;
    UINT16 i;
    
    u16TempAddr = u16Addr;
    
    for(i=ToRead; i>0; i--)
    {
      *Buffer++ = I2CMasterReadOneByte(u8DeviceAddr, u16TempAddr++);
    } 

 }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                RTC�ײ���������                                             //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*************************************************************************************************************
 * �������ƣ� I2C_GetAck
 * ���������� ��RTC��Ӧ��CMD���ж�������
 *
 * ��  �룺 
 * ��  ��   					
 ************************************************************************************************************/
UINT8 I2C_GetAck(void)
{
  BOOL bAck = 0;
  UINT8 z=100;

  I2CSCL_SET_0;
  I2C_Pins_DIR_Setting(0);
  I2CSCL_SET_1;

  wait:
  bAck = ((I2cSdaIn & I2cSda));
  if((bAck!=0)&&((z--)!=0))
  {
     goto wait;
  }
  I2CSCL_SET_0;
  delay5us();
  return(bAck);
}

/*************************************************************************************************************
 * �������ƣ� RtcWrite
 * ���������� ��RTC��������
 * ��  �룺 UINT8 u8Cmd, ָ��
 *          UINT8 *Buffer, 	
 *          UINT8 ToWrite	��buffer�ĸ���
 * ��  ����	NO_VALUABLE_DATA / I2C_OK 						
 * ȫ�ֱ�����
 * ����ģ�飺
 ************************************************************************************************************/
BOOL RtcWrite(UINT8 u8Cmd, UINT8 *Buffer, UINT8 ToWrite)
{
  UINT8 i=0;
  
  I2C_Pins_DIR_Setting(1);
  
  Engender_I2C_start_signal();
  I2C_WRITE_BYTE(RTC_WRITE_ADDR|u8Cmd);
  if(I2C_GetAck() & I2cSda)
  {
    Engender_I2C_stop_signal();
    return (0);
  }
  
  for(i=ToWrite; i>0; i--)
  {  
    I2C_Pins_DIR_Setting(1);
    I2C_WRITE_BYTE(*Buffer++);
 
    if(I2C_GetAck() & I2cSda)
    {
      Engender_I2C_stop_signal();
      return (0);
    }
  }
  
  Engender_I2C_stop_signal();
  I2CSCL_SET_0;
  I2CSDA_SET_0;    
  Delay_MS(15);
  
  return (1);
}

/*************************************************************************************************************
 * �������ƣ� RtcRead
 * ���������� ��RTC��Ӧ��CMD���ж�������
 * ��  �룺 UINT8 u8Cmd, ָ��
 *          UINT8 *Buffer, 	
 *          UINT8 ToRead	��buffer�ĸ���
 * ��  ����	NO_VALUABLE_DATA / I2C_OK 						
 * ȫ�ֱ�����
 * ����ģ�飺
 ************************************************************************************************************/
BOOL RtcRead(UINT8 u8Cmd, UINT8 *Buffer, UINT8 ToRead)
{
  UINT8 i=0;
  
  I2C_Pins_DIR_Setting(1);
  
  Engender_I2C_start_signal();
  I2C_WRITE_BYTE(RTC_READ_ADDR|u8Cmd);
  if(I2C_GetAck() & I2cSda)
  {
    Engender_I2C_stop_signal();
    return (0);
  }
  
  for(i=ToRead; i>1; i--)
  {  
    I2C_Pins_DIR_Setting(0);
    *Buffer++ = I2C_READ_BYTE();
    
    I2C_Pins_DIR_Setting(1);   
    Engender_I2C_ack_signal();
  }
  
  I2C_Pins_DIR_Setting(0);
  *Buffer = I2C_READ_BYTE();
  I2C_Pins_DIR_Setting(1);     
  Engender_I2C_noack_signal();
  Engender_I2C_stop_signal();
  I2CSCL_SET_0;
  I2CSDA_SET_0;    
  Delay_MS(15);
  
  return (1);
}


