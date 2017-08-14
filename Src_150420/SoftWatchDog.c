/******************************************************************************
*											
* Copyright (C) 2015, Sunic-ocean
* All Rights Reserved										
*
* 文 件 名： SoftWatchDog.c
* 创建时间： 2015年4月16日14:55:36
* 描    述： 软件定时器
*
*******************************************************************************/

#include "DataType.h"
#include "SoftWatchDog.h"
#include "Timer.h"

#define CRITICAL_ENTER()  TimerInterruptDisable()
#define CRITICAL_EXIT()   TimerInterruptEnable()

//每个软件定时器占用6个byte，使用越多，占用内存越多
#define SWD_MAX_SIZE      10

//软件定时器结构体声明
typedef struct _tsSwd
{
  UINT16 u16LifeTime;
  
  UINT16 u16LifeOldTime;
  
  BOOL bAvailabled;
  
  BOOL bEnable;
}_tsDogs;

_tsDogs sDogs[SWD_MAX_SIZE];

/******************************************************************************
 * 函数名称：SwdInit
 * 描述： 软件定时器初始化
 *
 * 参数：无
 * 返回：无
 *****************************************************************************/
void SwdInit(void)
{
  SINT8 i;
  
  CRITICAL_ENTER();
  for(i=0; i<SWD_MAX_SIZE; i++)
  {
    sDogs[i].u16LifeTime = 0;
    sDogs[i].u16LifeOldTime =0;
    
    sDogs[i].bAvailabled = TRUE;
    sDogs[i].bEnable = FALSE;
  }
  CRITICAL_EXIT();
}

/******************************************************************************
 * 函数名称：SwdGet
 * 描述： 得到一个可用的软件定时器
 *
 * 参数：无
 * 返回：可用定时器标号
 *****************************************************************************/
SINT8 SwdGet(void)
{
  SINT8 i;
  
  for(i=0; i<SWD_MAX_SIZE; i++)
  {
    if(sDogs[i].bAvailabled == TRUE)
    {
      sDogs[i].bAvailabled = FALSE;
      return i;
    }
  }
  
  return -1;
}

/******************************************************************************
 * 函数名称：SwdSetLifeTime
 * 描述： 设置软件定时器的初始时间
 *
 * 参数：sDog -- 定时器标号
 *       u16LifeTime -- 定时器的工作时间，定时器设置为10ms中断，本函数将设置的
                        存活时间进行衰减10倍处理
 * 返回：SWD_OK -- 设置成功
 *       SWD_ERROR -- 设置失败
 *****************************************************************************/
SWD_STATUS SwdSetLifeTime(SINT8 s8Dog, UINT16 u16LifeTime)
{
  if((s8Dog < 0) || (s8Dog >= SWD_MAX_SIZE))
  {
    return SWD_ERROR;
  }
  
  CRITICAL_ENTER();
  sDogs[s8Dog].u16LifeTime = u16LifeTime / 10;
  sDogs[s8Dog].u16LifeOldTime = u16LifeTime / 10;
  CRITICAL_EXIT();
  
  return SWD_OK;
}

/******************************************************************************
 * 函数名称：SwdGetLifeTime
 * 描述： 获取软件定时器的初始时间
 *
 * 参数：sDog -- 定时器标号
 *       u16LifeTime -- 定时器的工作时间
 * 返回：SWD_OK -- 设置成功
 *       SWD_ERROR -- 设置失败
 *****************************************************************************/
UINT16 SwdGetLifeTime(SINT8 s8Dog)
{
  UINT16 u16LifeTime=0;
  
  if((s8Dog < 0) || (s8Dog >= SWD_MAX_SIZE))
  {
    return 0xffff;
  }
  
  CRITICAL_ENTER();
  u16LifeTime = sDogs[s8Dog].u16LifeTime;
  CRITICAL_EXIT();
  
  return u16LifeTime;
}

/******************************************************************************
 * 函数名称：SwdEnable
 * 描述： 使能指定的软件定时器
 *
 * 参数：sDog -- 定时器标号
 * 返回：SWD_OK -- 设置成功
 *       SWD_ERROR -- 设置失败
 *****************************************************************************/
SWD_STATUS SwdEnable(SINT8 s8Dog)
{
  if((s8Dog < 0) || (s8Dog >= SWD_MAX_SIZE))
  {
    return SWD_ERROR;
  }
  
  CRITICAL_ENTER();
  sDogs[s8Dog].bEnable = TRUE;
  CRITICAL_EXIT();
  
  return SWD_OK;
}

/******************************************************************************
 * 函数名称：SwdIsEnable
 * 描述：指定的软件定时器是否使能
 *
 * 参数：sDog -- 定时器标号
 * 返回：1 -- 使能
 *       0 -- 关闭
 *****************************************************************************/
SINT8 SwdIsEnable(SINT8 s8Dog)
{
  BOOL bFlag=0;
  
  if((s8Dog < 0) || (s8Dog >= SWD_MAX_SIZE))
  {
    return SWD_ERROR;
  }
  
  CRITICAL_ENTER();
  bFlag = sDogs[s8Dog].bEnable;
  CRITICAL_EXIT();
  
  return bFlag;
}

/******************************************************************************
 * 函数名称：SwdDisable
 * 描述： 关闭指定的软件定时器
 *
 * 参数：sDog -- 定时器标号
 * 返回：SWD_OK -- 设置成功
 *       SWD_ERROR -- 设置失败
 *****************************************************************************/
SWD_STATUS SwdDisable(SINT8 s8Dog)
{
  if((s8Dog < 0) || (s8Dog >= SWD_MAX_SIZE))
  {
    return SWD_ERROR;
  }
  
  CRITICAL_ENTER();
  sDogs[s8Dog].bEnable = FALSE;
  CRITICAL_EXIT();
  
  return SWD_OK;
}

/******************************************************************************
 * 函数名称：SwdReset
 * 描述： 复位指定的定时器到初始值
 *
 * 参数：sDog -- 定时器标号
 * 返回：SWD_OK -- 设置成功
 *       SWD_ERROR -- 设置失败
 *****************************************************************************/
SWD_STATUS SwdReset(SINT8 s8Dog)
{
  if((s8Dog < 0) || (s8Dog >= SWD_MAX_SIZE))
  {
    return SWD_ERROR;
  }
  
  CRITICAL_ENTER();
  sDogs[s8Dog].u16LifeTime = sDogs[s8Dog].u16LifeOldTime;
  CRITICAL_EXIT();
  
  return SWD_OK;
}

/******************************************************************************
 * 函数名称：SwdFree
 * 描述： 释放指定的软件定时器
 *
 * 参数：sDog -- 定时器标号
 * 返回：SWD_OK -- 设置成功
 *       SWD_ERROR -- 设置失败
 *****************************************************************************/
SWD_STATUS SwdFree(SINT8 s8Dog)
{
  if((s8Dog < 0) || (s8Dog >= SWD_MAX_SIZE))
  {
    return SWD_ERROR;
  }
  
  CRITICAL_ENTER();
  sDogs[s8Dog].bAvailabled = TRUE;
  sDogs[s8Dog].bEnable = FALSE;
  sDogs[s8Dog].u16LifeTime = 0;
  sDogs[s8Dog].u16LifeOldTime = 0;
  CRITICAL_EXIT();
  
  return SWD_OK;
}

/******************************************************************************
 * 函数名称：SwdRun
 * 描述：运行软件定时器
 *
 * 参数：无
 * 返回：无
 *****************************************************************************/
void SwdRun(void)
{
  SINT8 i;
  
  for(i=0; i<SWD_MAX_SIZE; i++)
  {
    if(sDogs[i].bAvailabled == FALSE)
    {
      if(sDogs[i].bEnable == TRUE)
      {
        if(sDogs[i].u16LifeTime > 0)
        {
          sDogs[i].u16LifeTime--;
        }
      }
    }
  }
}