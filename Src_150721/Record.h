
#ifndef __RECORD_H__
#define __RECORD_H__

#include "Communication.h"

#ifdef  RECORD_GLOBAL
#define RECORD_EXT    
#else
#define RECORD_EXT extern
#endif

//存储设备器件地址
#define EE_WRITE_ADDR       0xa0
#define EE_READ_ADDR        0xa1

#define SYS_TYPE_WYL        BIT0
#define SYS_TYPE_SYJ        BIT1
    
//保存使能标志位
#define SAVE_NONE           0x00
#define SAVE_SYS_NBR        BIT0
#define SAVE_SYS_TYPE       BIT1
#define SAVE_SYS_GAP        BIT2
#define SAVE_REV_BD_NBR     BIT3
#define SAVE_BCK_BD_NBR     BIT4
#define SAVE_REV_CDMA_NBR   BIT5
#define SAVE_BCK_CDMA_NBR   BIT6
#define SAVE_ALL_CFG        (BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0)

//定义各存储地址，预留0x00 ~ 0x0f
#define SYS_CFG_ADDR        0x00    //初次设置配置，0xff为设备没有被配置，0x55为已经被配置
#define SYS_NBR_ADDR        0x10    //设备编码
#define SYS_TYPE_ADDR       0x20    //设备类型
#define SYS_GAP_ADDR        0x21    // 发送时间间隔保存
#define SYS_REV_BD_ADDR     0x30    //北斗接收机号码
#define SYS_BCK_BD_ADDR     0x30    //备用北斗接收机号码
#define SYS_REV_CDMA_ADDR   0x40    //cdma接收机号码
#define SYS_BCK_CDMA_ADDR   0x50    //备用接收机号码
#define SYS_RUN_TIME_ADDR   0x60    //程序运行时间，UINT32类型，单位为分钟数
#define SYS_BD_TX_ADDR      0x70    //北斗发送报文次数
#define SYS_BD_RX_ADDR      0x72    //北斗接收报文次数
#define SYS_BD_TX_SUCC      0x74    //北斗发送成功次数

typedef struct
{
  UINT16 u16SysNbr;         //设备编号
  UINT8  u8SysType;         //设备类型
  UINT8  u8GapTime;         //信息发送时间间隔，最高间隔时间255/60= 4.25H
  
  UINT8  au8BdRevNbr[3];    //北斗接收机号码指针
  UINT8  au8BdBckNbr[3];    //北斗备用接收机号码指针
  
  UINT8  au8CdmaRevNbr[11];       //CDMA接收机号码指针
  UINT8  au8CdmaBckNbr[11];       //cdma备用接收机号码指针
  
  UINT32  u32SysRunTime;          //程序运行时间
}_tsRecordCb;

RECORD_EXT _tsRecordCb sRecordCb;


/********************************************************************************
 * 函数名称：RecordInit.c
 * 函数功能：初始化记录信息，包括设备编号，设备信息等
 * 
 * 输入： 无
 * 返回： 无
 *  
 *******************************************************************************/
extern void RecordInit(void);

/********************************************************************************
 * 函数名称：RecordRun
 * 函数功能：接收串口发送的配置数据，并显示在显示器上
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
extern void RecordRun(void);

/********************************************************************************
 * 函数名称：ReadSystemNbr
 * 函数功能：读取设备的编码
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
extern void ReadSystemNbr(UINT16 *ptr);

/********************************************************************************
 * 函数名称：ReadSystemType
 * 函数功能：读取设备的类型
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
extern void ReadSystemType(UINT8 *ptr);

/********************************************************************************
 * 函数名称：ReadBdReceivingNbr
 * 函数功能：读取北斗接收机号码
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
extern void ReadBdReceivingNbr(UINT8 *ptr);

/********************************************************************************
 * 函数名称：ReadBckBdReceivingNbr
 * 函数功能：读取备用北斗接收机号码
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
extern void ReadBckBdReceivingNbr(UINT8 *ptr);

/********************************************************************************
 * 函数名称：ReadCdmaReceivingNbr
 * 函数功能：读取CDMA接收机号码
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
extern void ReadCdmaReceivingNbr(UINT8 *prt);

/********************************************************************************
 * 函数名称：ReadBckCdmaReceivingNbr
 * 函数功能：读取备用CDMA接收机号码
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
extern void ReadBckCdmaReceivingNbr(UINT8 *ptr);

/********************************************************************************
 * 函数名称：SaveSystemNbr
 * 函数功能：保存系统编号
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
 extern void SaveSystemNbr(UINT16 *ptr);
 
 /********************************************************************************
 * 函数名称：SaveSystemNbr
 * 函数功能：保存设备类型
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
 extern void SaveSystemType(UINT8 *ptr);
 
 /********************************************************************************
 * 函数名称：SaveBdReceivingNbr
 * 函数功能：保存北斗接收机号码
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
 extern void SaveBdReceivingNbr(UINT8 *ptr);
 
 /********************************************************************************
 * 函数名称：SaveBckBdReceivingNbr
 * 函数功能：保存备用北斗接收机号码
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
 extern void SaveBckBdReceivingNbr(UINT8 *ptr);

/********************************************************************************
 * 函数名称：SaveBdReceivingNbr
 * 函数功能：保存cdma接收机号码
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
 extern void SaveCdmaReceivingNbr(UINT8 *ptr);
 
 /********************************************************************************
 * 函数名称：SaveBckCdmaReceivingNbr
 * 函数功能：备用cdma接收机号码
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/ 
 extern void SaveBckCdmaReceivingNbr(UINT8 *ptr);
 
/********************************************************************************
 * 函数名称：SaveSystemRunTime
 * 函数功能：保存程序运行时间
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
extern void SaveSystemRunTime(UINT32 *ptr);

/********************************************************************************
 * 函数名称：ReadSystemRunTime
 * 函数功能：读取程序上次运行时间
 *
 * 输入： 无
 * 返回： 无
 *
 *******************************************************************************/
extern void ReadSystemRunTime(UINT32 *ptr);

#ifdef BEIDOU_TEST
extern void ReadBdRxNbr(UINT16 *ptr);
extern void ReadBdSendNbr(UINT16 *ptr);
extern void ReadBdSendSucceedNbr(UINT16 *ptr);

extern void SaveBdRxNbr(UINT16 *ptr);
extern void SaveBdSendNbr(UINT16 *ptr);
extern void SaveBdSendSucceedNbr(UINT16 *ptr);
#endif

#endif