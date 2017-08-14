
#ifndef __RECORD_H__
#define __RECORD_H__

//设备信息存储地址
#define SYSTEM_STATE_ADDR       0x00
#define TERMINAL_INDEX_ADDR     0x01
#define TERMINAL_TYPE_ADDR      0x05
#define TERMINAL_INFO_LEN       0x06
#define TERMINAL_INFO_ADDR      0x07
#define TIME_SPACE_ADDR         0x30

//DTU标识号码
#define DTU_INDEX_ADDR          0x31   //11个字节空间

//软件版本号
#define FW_VER_ADDR             0x3c   //固件版本，前后4byte分别存储
#define DEBUG_EN_ADDR           0x3d   //debug开关。1 --- on；0 ---off

//起始地址存放端口号
#define DTU_MAIN_COM_ADDR       0x40
#define DTU_MAIN_TCP_ADDR       0x42
#define DTU_MAIN_APN_ADDR       0x43
#define DTU_MAIN_NAME_LEN       0x44
#define DTU_MAIN_NAME_ADDR      0x45  //用户名为最大16个字符
#define DTU_MAIN_PASS_LEN       0x55
#define DTU_MAIN_PASS_ADDR      0x56  //密码最大为16个字符
#define DTU_MAIN_LEN_ADDR       0x66
#define DTU_MAIN_IP_ADDR        0x67

//起始地址开始先存入两个BYTE的端口号，接下来是IP或者域名
//域名最大长度为60
#define DTU_SUB1_COM_ADDR       0xB0
#define DTU_SUB1_EN_ADDR        0xB2
#define DTU_SUB1_LEN_ADDR       0xB3
#define DTU_SUB1_IP_ADDR        0xB4

#define DTU_SUB2_COM_ADDR       0xF0
#define DTU_SUB2_EN_ADDR        0xF2
#define DTU_SUB2_LEN_ADDR       0xF3
#define DTU_SUB2_IP_ADDR        0xF4

#define DTU_SUB3_COM_ADDR       0x130
#define DTU_SUB3_EN_ADDR        0x132
#define DTU_SUB3_LEN_ADDR       0x133
#define DTU_SUB3_IP_ADDR        0x134

#define DTU_SUB4_COM_ADDR       0x170
#define DTU_SUB4_EN_ADDR        0x172
#define DTU_SUB4_LEN_ADDR       0x173
#define DTU_SUB4_IP_ADDR        0x174



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


//检查正常开关机状态
extern BOOL CheckPowerDown(void);

#endif