

#ifndef __SYSTEMCTL_H__
#define __SYSTEMCTL_H__

/*****************************************************************************
 函 数 名  : SystemVersionInfo
 功能描述  : 打印系统版本信息
 输入参数  : 
 返 回 值  :
*****************************************************************************/
extern void SystemVersionInfo(void);

/*****************************************************************************
 函 数 名  : InitClk
 功能描述  : 系统时钟初始化，设置为4194304HZ
 输入参数  : 
 输出参数  : NONE
 返 回 值  :
*****************************************************************************/
extern void InitClk(void);

/******************系统初始化******************/
extern void SystemInit(void);

/*****************************************************************************
 函 数 名  : WaitSystemStart
 功能描述  : 系统开机控制

  参  数  : NONE
 返 回 值 : NONE
*****************************************************************************/
extern void WaitSystemStart(void);

extern void SystemIdleInit(void);
extern void SystemIdleRun(void);

//把主时钟和子时钟由DCO切换到XT1
extern void DcoSwitchXt1(void);

/****************************************************************
 * 函数名称：MemClr
 * 功    能：内存设置为0
 *
 * 参    数：UINT8 *pu8Buf,  UINT8 u8Len
 * 返回值  ：无
****************************************************************/
extern void MemClr(UINT8 *pu8Buf,  UINT8 u8Len);

/****************************************************************
 * 函数名称：MEM_CPY
 * 功    能：将BUF2中内容复制到BUF1中
 *
 * 参    数：UINT8 *pu8Buf1, UINT8 *pu8Buf2, UINT8 u8Len
 * 返回值  ：无
****************************************************************/
extern void MEM_CPY(void *pu8Buf1, const void *pu8Buf2, UINT8 u8Len);

/****************************************************************
 * 函数名称：MEM_CMP
 * 功    能：内存比较。
 *
 * 参    数：const void *cs, const void *ct, UINT8 count
 * 返回值  ：SINT8
****************************************************************/
extern SINT8 MEM_CMP(const void *cs, const void *ct, UINT8 count);


#endif