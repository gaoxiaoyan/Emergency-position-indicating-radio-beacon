/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                            (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
* File : uCOS_II.H
* By   : Jean J. Labrosse
*********************************************************************************************************
*/

#ifndef __OS_MEM_H__
#define __OS_MEM_H__

#include "os_cfg.h"
#include "DataType.h"
#include "msp430x54x.h"

#ifdef   OS_GLOBALS
#define  OS_EXT
#else
#define  OS_EXT  extern
#endif

#define OS_ENTER_CRITICAL()   _DINT()     
#define OS_EXIT_CRITICAL()    _EINT()     

/*
*********************************************************************************************************
*                                             ERROR CODES
*********************************************************************************************************
*/
#define OS_NO_ERR                 0

#define OS_MEM_INVALID_PART     110
#define OS_MEM_INVALID_BLKS     111
#define OS_MEM_INVALID_SIZE     112
#define OS_MEM_NO_FREE_BLKS     113
#define OS_MEM_FULL             114
#define OS_MEM_INVALID_PBLK     115
#define OS_MEM_INVALID_PMEM     116
#define OS_MEM_INVALID_PDATA    117
#define OS_MEM_INVALID_ADDR     118

/*$PAGE*/

/*
*********************************************************************************************************
*                                     MEMORY PARTITION DATA STRUCTURES
*********************************************************************************************************
*/

#if (OS_MEM_EN > 0) && (OS_MAX_MEM_PART > 0)
typedef struct {                       /* MEMORY CONTROL BLOCK                                         */
    void   *OSMemAddr;                 /* Pointer to beginning of memory partition                     */
    void   *OSMemFreeList;             /* Pointer to list of free memory blocks                        */
    INT32U  OSMemBlkSize;              /* Size (in bytes) of each block of memory                      */
    INT32U  OSMemNBlks;                /* Total number of blocks in this partition                     */
    INT32U  OSMemNFree;                /* Number of memory blocks remaining in this partition          */
} OS_MEM;


typedef struct {
    void   *OSAddr;                    /* Pointer to the beginning address of the memory partition     */
    void   *OSFreeList;                /* Pointer to the beginning of the free list of memory blocks   */
    INT32U  OSBlkSize;                 /* Size (in bytes) of each memory block                         */
    INT32U  OSNBlks;                   /* Total number of blocks in the partition                      */
    INT32U  OSNFree;                   /* Number of memory blocks free                                 */
    INT32U  OSNUsed;                   /* Number of memory blocks used                                 */
} OS_MEM_DATA;
#endif

/*$PAGE*/

/*$PAGE*/
/*
*********************************************************************************************************
*                                            GLOBAL VARIABLES
*********************************************************************************************************
*/
#if (OS_MEM_EN > 0) && (OS_MAX_MEM_PART > 0)
OS_EXT  OS_MEM           *OSMemFreeList;            /* Pointer to free list of memory partitions       */
OS_EXT  OS_MEM            OSMemTbl[OS_MAX_MEM_PART];/* Storage for memory partition manager            */
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*                                     (Target Independent Functions)
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           MEMORY MANAGEMENT
*********************************************************************************************************
*/

#if (OS_MEM_EN > 0) && (OS_MAX_MEM_PART > 0)

OS_MEM       *OSMemCreate(void *addr, INT32U nblks, INT32U blksize, INT8U *err);
void         *OSMemGet(OS_MEM *pmem, INT8U *err);
INT8U         OSMemPut(OS_MEM *pmem, void *pblk);
void          OS_MemInit (void);

#if OS_MEM_QUERY_EN > 0
INT8U         OSMemQuery(OS_MEM *pmem, OS_MEM_DATA *pdata);
#endif

#endif



/*$PAGE*/
/*
*********************************************************************************************************
*                                      INTERNAL FUNCTION PROTOTYPES
*                            (Your application MUST NOT call these functions)
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           MEMORY MANAGEMENT
*********************************************************************************************************
*/

#ifndef OS_MEM_EN
#error  "OS_CFG.H, Missing OS_MEM_EN: Enable (1) or Disable (0) code generation for MEMORY MANAGER"
#else
    #ifndef OS_MAX_MEM_PART
    #error  "OS_CFG.H, Missing OS_MAX_MEM_PART: Max. number of memory partitions"
    #else
        #if     OS_MAX_MEM_PART == 0
        #error  "OS_CFG.H, OS_MAX_MEM_PART must be > 0"
        #endif
        #if     OS_MAX_MEM_PART > 255
        #error  "OS_CFG.H, OS_MAX_MEM_PART must be <= 255"
        #endif
    #endif

    #ifndef OS_MEM_QUERY_EN
    #error  "OS_CFG.H, Missing OS_MEM_QUERY_EN: Include code for OSMemQuery()"
    #endif
#endif


/*
*********************************************************************************************************
*                                           MEMORY BLOCK STATEMENT
*********************************************************************************************************
*/
#define BUF_LARGE_SIZE         1024
#define BUF_MEDIUM_SIZE        512
#define BUF_SMALL_SIZE         256

OS_EXT __no_init UINT8 au8LargeMem[4][BUF_LARGE_SIZE];
OS_EXT __no_init UINT8 au8MediumMem[6][BUF_MEDIUM_SIZE];
OS_EXT __no_init UINT8 au8SmallMem[12][BUF_SMALL_SIZE];
OS_EXT UINT8 err;
OS_EXT UINT8 *pMemErr;
OS_EXT OS_MEM *pLargeMem;
OS_EXT OS_MEM *pMediumMem;
OS_EXT OS_MEM *pSmallMem;

#endif