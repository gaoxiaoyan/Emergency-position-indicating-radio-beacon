
/******************************************************************************************************
 *											
 * Copyright (C) 2014, Sunic-ocean
 * All Rights Reserved										
 *
 * 文 件 名： SystemCtl.c
 * 描       述：

 * 创建日期： 2014年9月4日 13:23:50
 * 作    者： Bob
 * 当前版本： V1.00
 *******************************************************************************************************/
#include "msp430x54x.h"

#include "global.h"
#include "DataType.h"
#include "Timer.h"
#include "SystemCtl.h"
#include "string.h"
#include "Uart3.h"
#include "SoftWatchDog.h"

#define CPU_CLK       4194304ul 
#define FLL_FACTOR    (CPU_CLK / 32768ul)


/*****************************************************************************
 *函数名称: InitClk
 *功能描述: 系统时钟初始化

 *参数: 
 *返回:
*****************************************************************************/
void InitClk(void) 
{     
    P11DS  |= BIT0 + BIT1 + BIT2;
    P11SEL |= BIT0 + BIT1 + BIT2;
    P11DIR |= BIT0 + BIT1 + BIT2;
    
    // 启动XT1  
    P7SEL |= BIT0 + BIT1;                        // P7.0 P7.1 外设功能  
    UCSCTL6 &= ~(XT1OFF);                       // XT1打开  
    UCSCTL6 |= XCAP_3;                          // 内部电容  
    do  
    {  
        UCSCTL7 &= ~XT1LFOFFG;                  // 清楚XT1错误标记  
    }while (UCSCTL7&XT1LFOFFG);                 // 检测XT1错误标记  

    __bis_SR_register(SCG0);                    // 禁止FLL功能  
    UCSCTL0 = 0x0000;                           // Set lowest possible DCOx, MODx  
    UCSCTL1 = DCORSEL_5;                        // DCO最大频率为6MHz  
    UCSCTL2 = FLLD_1 + FLL_FACTOR;              // 设置DCO频率为4MHz  

    __bic_SR_register(SCG0);                    // 使能FLL功能  
  
    // 必要延时  
    __delay_cycles(250000);  
  
    // 清楚错误标志位  
    do  
    {  
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);  
                                                // 清除所有振荡器错误标志位  
        SFRIFG1 &= ~OFIFG;                      // 清除振荡器错误  
    }while (SFRIFG1&OFIFG);                     // 等待清楚完成  
    
    UCSCTL4 |= SELS_7 + SELM_7;
}  

/*****************************************************************************
 *函数名称: SystemVersionInfo
 *功能描述: 打印系统版本信息

 *参数: 
 *返回:
*****************************************************************************/
void SystemVersionInfo(void)
{
  UINT8 au8Version[8];

  if(g_bDebug == TRUE)
  {
    Uart3SendString("\r\n #Debug HandShake OK! The Bandrate is 115200bps.\r\n");
    Uart3SendString("/***********************************************************/\r\n");
    Uart3SendString("// Copyright (C) 2015, Sunic-ocean.\r\n");
    Uart3SendString("// All Rights Reserved.\r\n");
    Uart3SendString(" \r\n");
    Uart3SendString("// 工程名称：应急物资示位标.\r\n");
      
    memset(&au8Version[0], 0, 8);
    au8Version[0] = 'V';
    au8Version[1] = VERSION / 100 + '0';
    au8Version[2] = '.';
    au8Version[3] = VERSION % 100 / 10 + '0';
    au8Version[4] = VERSION % 10 + '0';
    au8Version[5] = '\r';
  
    Uart3SendString("// 版本信息：");
    Uart3SendString(&au8Version[0]);
    Uart3SendString("/***********************************************************/\r\n");
    Uart3SendString(" \r\n");
  }
}

/*****************************************************************************
 *函数名称: SystemInit
 *功能描述: 系统初始化

 *参数: 
 *返回:
*****************************************************************************/
void SystemInit(void)
{
  
  //程序运行指示灯
  SystemRunDisable();
  SystemRunDir |= SystemRun;
  
  //低压指示灯
  LowPowerLedDisable();
  LowPowerLedDir |= LowPowerLed;
 
 	//围油栏指示灯
 	WylLedDisable();
 	WylLedDir |= WylLed;
 	
 	//收油机指示灯
 	SyjLedDisable();
 	SyjLedDir |= SyjLed;
 	
 	//开关机按键接口, 下降沿触发中断
 	KeyPowerIesEdge();
 	KeyPowerIeEnable();
 	KeyPowerDir &= ~KeyPower;
 	
 	//RTC中断，上升沿触发中断
 	RtcIntIesEdge();
 	RtcIntIeEnable();
 	RtcIntDir &= ~RtcInt;
 	
 	//CDMA RI中断，下降沿触发中断
 	CdmaRiIesEdge();
 	CdmaRiIntEnable();
 	CdmaRiDir &= ~CdmaRi;
 	
 	//GPS电源控制接口
 	GpsPowerOff();
 	GpsPowerDir |= GpsPower;
 	
 	//CDMA电源控制接口
 	CdmaPowerOff();
 	CdmaPowerDir |= CdmaPower;
 	
 	//BD电源控制接口
 	BeiDouPowerOff();
 	BeiDouPowerDir |= BeiDouPower;
 	
 	//自锁电源控制接口
 	SystemPowerOff();
 	SystemPowerDir |= SystemPower;
 	
}

/*****************************************************************************
 *函数名称: WaitSystemStart
 *功能描述: 系统开机检查

 *参数: 
 *返回:
*****************************************************************************/
void WaitSystemStart(void)
{
  SINT8 s8Dog;
  
  s8Dog = SwdGet();
  SwdSetLifeTime(s8Dog, 300);
  SwdEnable(s8Dog);
  
  while(TRUE)
  {
    if(!KeyPowerCheck)
    {
      if(SwdGetLifeTime(s8Dog) == 0)
      {
        SwdDisable(s8Dog);
        SwdFree(s8Dog);
        
        SystemPowerOn();
        GpsPowerOn();
        CdmaPowerOn();
        BeiDouPowerOn();
        
        return;
      }
      else
      {
        SystemPowerOff();
      }
    }
  }
}


/*****************************************************************************
 函 数 名  : PORT1_ISR
 功能描述  : P1端口的中断服务函数

  参  数  : NONE
 返 回 值 : NONE
*****************************************************************************/
#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
  static UINT8 u8Temp=0;
  
  u8Temp++;
}

/****************************************************************
 * 函数名称：MEM_SET
 * 功    能：内存设置为固定值
 *
 * 参    数：UINT8 *pu8Buf, UINT8 u8Set, UINT8 u8Len
 * 返回值  ：无
****************************************************************/
void MEM_SET(void *pu8Buf, UINT8 u8Set, UINT8 u8Len)
{
  UINT8 *p = (UINT8 *)pu8Buf;
  
	while(u8Len--)
	{
		*p++ = u8Set;
	}
}

/****************************************************************
 * 函数名称：MEM_CPY
 * 功    能：将BUF2中内容复制到BUF1中
 *
 * 参    数：UINT8 *pu8Buf1, UINT8 *pu8Buf2, UINT8 u8Len
 * 返回值  ：无
****************************************************************/
void MEM_CPY(void *pu8Buf1, const void *pu8Buf2, UINT8 u8Len)
{
  void *dst = pu8Buf1;
  
  while(u8Len-- > 0)
  {
    *(UINT8 *)dst = *(UINT8 *)pu8Buf2;
     dst = (UINT8 *)dst + 1;
     pu8Buf2 = (UINT8 *)pu8Buf2 + 1;
  }

}

/****************************************************************
 * 函数名称：MEM_CMP
 * 功    能：内存比较。
 *
 * 参    数：const void *cs, const void *ct, UINT8 count
 * 返回值  ：SINT8
****************************************************************/
SINT8 MEM_CMP(const void *cs, const void *ct, UINT8 count)
{
  const UINT8 *su1, *su2;
  SINT8 res = 0;
        
  for (su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
  {
    if ((res = *su1 - *su2) != 0)
    {
      break;
    }
  }
  return res;
}


