/******************************************************************************
*											
* Copyright (C) 2015, Sunic-ocean
* All Rights Reserved										
*
* �� �� ���� SoftWatchDog.c
* ����ʱ�䣺 2015��4��16��14:55:36
* ��    ���� �����ʱ��
*
*******************************************************************************/

#include "DataType.h"
#include "SoftWatchDog.h"
#include "Timer.h"

#define CRITICAL_ENTER()  TimerInterruptDisable()
#define CRITICAL_EXIT()   TimerInterruptEnable()

//ÿ�������ʱ��ռ��6��byte��ʹ��Խ�࣬ռ���ڴ�Խ��
#define SWD_MAX_SIZE      10

//�����ʱ���ṹ������
typedef struct _tsSwd
{
  UINT16 u16LifeTime;
  
  UINT16 u16LifeOldTime;
  
  BOOL bAvailabled;
  
  BOOL bEnable;
}_tsDogs;

_tsDogs sDogs[SWD_MAX_SIZE];

/******************************************************************************
 * �������ƣ�SwdInit
 * ������ �����ʱ����ʼ��
 *
 * ��������
 * ���أ���
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
 * �������ƣ�SwdGet
 * ������ �õ�һ�����õ������ʱ��
 *
 * ��������
 * ���أ����ö�ʱ�����
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
 * �������ƣ�SwdSetLifeTime
 * ������ ���������ʱ���ĳ�ʼʱ��
 *
 * ������sDog -- ��ʱ�����
 *       u16LifeTime -- ��ʱ���Ĺ���ʱ�䣬��ʱ������Ϊ10ms�жϣ������������õ�
                        ���ʱ�����˥��10������
 * ���أ�SWD_OK -- ���óɹ�
 *       SWD_ERROR -- ����ʧ��
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
 * �������ƣ�SwdGetLifeTime
 * ������ ��ȡ�����ʱ���ĳ�ʼʱ��
 *
 * ������sDog -- ��ʱ�����
 *       u16LifeTime -- ��ʱ���Ĺ���ʱ��
 * ���أ�SWD_OK -- ���óɹ�
 *       SWD_ERROR -- ����ʧ��
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
 * �������ƣ�SwdEnable
 * ������ ʹ��ָ���������ʱ��
 *
 * ������sDog -- ��ʱ�����
 * ���أ�SWD_OK -- ���óɹ�
 *       SWD_ERROR -- ����ʧ��
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
 * �������ƣ�SwdIsEnable
 * ������ָ���������ʱ���Ƿ�ʹ��
 *
 * ������sDog -- ��ʱ�����
 * ���أ�1 -- ʹ��
 *       0 -- �ر�
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
 * �������ƣ�SwdDisable
 * ������ �ر�ָ���������ʱ��
 *
 * ������sDog -- ��ʱ�����
 * ���أ�SWD_OK -- ���óɹ�
 *       SWD_ERROR -- ����ʧ��
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
 * �������ƣ�SwdReset
 * ������ ��λָ���Ķ�ʱ������ʼֵ
 *
 * ������sDog -- ��ʱ�����
 * ���أ�SWD_OK -- ���óɹ�
 *       SWD_ERROR -- ����ʧ��
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
 * �������ƣ�SwdFree
 * ������ �ͷ�ָ���������ʱ��
 *
 * ������sDog -- ��ʱ�����
 * ���أ�SWD_OK -- ���óɹ�
 *       SWD_ERROR -- ����ʧ��
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
 * �������ƣ�SwdRun
 * ���������������ʱ��
 *
 * ��������
 * ���أ���
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