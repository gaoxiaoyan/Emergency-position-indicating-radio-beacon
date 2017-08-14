
#ifndef __IIC_DRIVER_H__
#define __IIC_DRIVER_H__

 #include "DataType.h"

// #define I2C_SCL				PCF8576_SCL
// #define I2C_SDA				PCF8576_SDA

 #define NO_VALUABLE_DATA  		0
 #define I2C_OK				1

 #define SLAVE_DEVICE_ERROR		0
 #define SLAVE_DEVICE_OK		1

//I2C�˿ڶ���
#define I2cSda                BIT5
#define I2cSdaOut             P7OUT
#define I2cSdaIn              P7IN
#define I2cSdaDir             P7DIR

#define I2cScl                BIT4
#define I2cSclOut             P7OUT
#define I2cSclDir             P7DIR

#define EepromPowerOn()          P6OUT |=  BIT6
#define EepromPowerOff()         P6OUT &= ~BIT6

 /*
  ��ʼ������EEPROM��Դ
 */
extern void I2CMasterWriteOneByte(UINT8 u8DeviceAddr, UINT16 u16Address, UINT8 u8Date);
extern UINT8 I2CMasterReadOneByte(UINT8 u8DeviceAddr, UINT16 u16Address);
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
 extern void I2CMasterWrite(UINT8 u8DeviceAddr, UINT16 u16Addr, UINT8 *Buffer, UINT8 ToWrite);
 
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
 extern void I2CMasterRead(UINT8 u8DeviceAddr, UINT16 u16Addr, UINT8 *Buffer, UINT8 ToRead);
 
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
extern BOOL RtcWrite(UINT8 u8Cmd, UINT8 *Buffer, UINT8 ToWrite);
 
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
extern BOOL RtcRead(UINT8 u8Cmd, UINT8 *Buffer, UINT8 ToRead);

#endif