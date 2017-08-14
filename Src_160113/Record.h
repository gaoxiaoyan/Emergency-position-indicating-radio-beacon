
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

#define SYS_TYPE_WYL        0x01
#define SYS_TYPE_SYJ        0x02

//保存使能标志位
#define SAVE_NONE           0x00    //无保存数据
#define SAVE_SYS_NBR        BIT0    //保存厂家标识和设备编号
#define SAVE_SYS_TYPE       BIT1    //保存设备类型
#define SAVE_SYS_GAP        BIT2    //保存设备时间间隔
#define SAVE_BD_NBR         BIT3    //保存北斗号码.包括客户接收和青岛中心
//#define SAVE_BCK_BD_NBR     BIT4
#define SAVE_CDMA_NBR       BIT4    //保存CDMA卡号，包括客户接收和青岛中心
//#define SAVE_BCK_CDMA_NBR   BIT6
#define SAVE_ALL_CFG        (BIT4 | BIT3 | BIT2 | BIT1 | BIT0)

//通讯模式
#define CDMA_COM_MODE       BIT1    //只有CDMA模式通讯
#define BD_COM_MODE         BIT0    //只有北斗模式通讯
#define AUTO_COM_MODE       (CDMA_COM_MODE | BD_COM_MODE) //北斗和cdma双模通讯

//定义各存储地址，预留0x00 ~ 0x07
#define SYS_CFG_ADDR        0x00    //初次设置配置，0xff为设备没有被配置，0x55为已经被配置
#define SYS_MOD_ADDR        0x08    //手动和自动模式切换。手动模式中，通讯模块不能断电，一直处于工作中。
#define SYS_COM_SEL_ADDR    0x09    //通讯方式选取。 3 -- 根据信号强度自动选取； 1 -- 选择CDMA模块； 2 -- 选择北斗模块。
#define SYS_CMP_ADDR        0x10    //公司编号
#define SYS_NBR_ADDR        0x18    //设备编号
#define SYS_TYPE_ADDR       0x20    //设备类型
#define SYS_GAP_ADDR        0x21    //发送时间间隔保存
#define SYS_REV_BD_ADDR     0x30    //北斗接收机号码
#define SYS_BCK_BD_ADDR     0x38    //备用北斗接收机号码
#define SYS_REV_CDMA_ADDR   0x40    //cdma接收机号码
#define SYS_BCK_CDMA_ADDR   0x50    //备用接收机号码
#define SYS_RUN_TIME_ADDR   0x60    //程序运行时间，UINT32类型，单位为分钟数
#define SYS_LOC_CDMA_ADDR   0x64    //CDMA发送机号码
#define SYS_BD_TX_ADDR      0x70    //北斗发送报文次数
#define SYS_BD_RX_ADDR      0x72    //北斗接收报文次数
#define SYS_BD_TX_SUCC      0x74    //北斗发送成功次数

typedef struct
{
  UINT16 u16CmpNbr;               //厂家编号, 0 ~ 65535.
  UINT16 u16SysNbr;               //设备编号, 0 ~ 65535.

  UINT8  u8SysType;               //设备类型, 0~255个类型；目前使用1 -- 收油机，2 -- 围油栏

  UINT8  u8GapTime;               //信息发送时间间隔，最高间隔时间255/60= 4.25H
                                  //默认为数字为5（分钟）。

  UINT8  au8BdRevNbr[3];          //北斗接收机号码，指向客户中心。
  UINT8  au8BdBckNbr[3];          //北斗备用接收机号码。预留调试，指向青岛中心北斗测试机

  UINT8  au8CdmaRevNbr[11];       //CDMA接收机号码
  UINT8  au8CdmaBckNbr[11];       //cdma备用接收机号码。预留测试，指向青岛中心短信猫测试机。

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

extern void ReadSystemBdIndex(UINT8 *ptr, UINT8 *pSource);

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

//测试北斗自收发成功率
#ifdef BEIDOU_TEST
extern void ReadBdRxNbr(UINT16 *ptr);
extern void ReadBdSendNbr(UINT16 *ptr);
extern void ReadBdSendSucceedNbr(UINT16 *ptr);

extern void SaveBdRxNbr(UINT16 *ptr);
extern void SaveBdSendNbr(UINT16 *ptr);
extern void SaveBdSendSucceedNbr(UINT16 *ptr);
#endif

#endif