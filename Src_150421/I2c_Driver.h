
#ifndef __IIC_DRIVER_H__
#define __IIC_DRIVER_H__

 #include "DataType.h"

// #define I2C_SCL				PCF8576_SCL
// #define I2C_SDA				PCF8576_SDA

 #define NO_VALUABLE_DATA  		0
 #define I2C_OK				1

 #define SLAVE_DEVICE_ERROR		0
 #define SLAVE_DEVICE_OK		1

//I2C端口定义
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
  初始化，打开EEPROM电源
 */
extern void I2CMasterWriteOneByte(UINT8 u8DeviceAddr, UINT16 u16Address, UINT8 u8Date);
extern UINT8 I2CMasterReadOneByte(UINT8 u8DeviceAddr, UINT16 u16Address);
/*************************************************************************************************************
 * 函数名称： I2CMasterWrite
 * 功能描述： I2C总线 写从机数据
 * 输  入： UINT8 u8Add, 从机地址
 *          UINT8 *Buffer, 	
 *          UINT8 ToWrite	写buffer的个数
 * 输  出：	NO_VALUABLE_DATA / I2C_OK 
 * 全局变量：
 * 调用模块：
 ************************************************************************************************************/
 extern void I2CMasterWrite(UINT8 u8DeviceAddr, UINT16 u16Addr, UINT8 *Buffer, UINT8 ToWrite);
 
 /*************************************************************************************************************
 * 函数名称： I2CMasterRead
 * 功能描述： I2C总线 写从机数据
 * 输  入： UINT8 SlaveAdd, 从机地址
 *          UINT8 *Buffer, 	
 *          UINT8 ToRead	读buffer的个数
 * 输  出：	NO_VALUABLE_DATA / I2C_OK 						
 * 全局变量：
 * 调用模块：
 ************************************************************************************************************/
 extern void I2CMasterRead(UINT8 u8DeviceAddr, UINT16 u16Addr, UINT8 *Buffer, UINT8 ToRead);
 
 /*************************************************************************************************************
 * 函数名称： RtcWrite
 * 功能描述： 对RTC进行设置
 * 输  入： UINT8 u8Cmd, 指令
 *          UINT8 *Buffer, 	
 *          UINT8 ToWrite	读buffer的个数
 * 输  出：	NO_VALUABLE_DATA / I2C_OK 						
 * 全局变量：
 * 调用模块：
 ************************************************************************************************************/
extern BOOL RtcWrite(UINT8 u8Cmd, UINT8 *Buffer, UINT8 ToWrite);
 
 /*************************************************************************************************************
 * 函数名称： RtcRead
 * 功能描述： 从RTC对应的CMD中中读出数据
 * 输  入： UINT8 u8Cmd, 指令
 *          UINT8 *Buffer, 	
 *          UINT8 ToRead	读buffer的个数
 * 输  出：	NO_VALUABLE_DATA / I2C_OK 						
 * 全局变量：
 * 调用模块：
 ************************************************************************************************************/
extern BOOL RtcRead(UINT8 u8Cmd, UINT8 *Buffer, UINT8 ToRead);

#endif