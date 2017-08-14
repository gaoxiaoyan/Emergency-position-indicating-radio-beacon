/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                           (c) Copyright 1992-2003, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
*                                  uC/OS-II Configuration File for V2.7x
*
* File    : OS_CFG.H
* By      : Jean J. Labrosse
* Version : V2.76
*********************************************************************************************************
*/

#ifndef OS_CFG_H
#define OS_CFG_H

                                       /* ---------------------- MISCELLANEOUS ----------------------- */
#define OS_ARG_CHK_EN             1    /* Enable (1) or Disable (0) argument checking                  */

#define OS_DEBUG_EN               1    /* Enable(1) debug variables                                    */

#define OS_MAX_MEM_PART           5    /* Max. number of memory partitions                             */

                                       /* --------------------- MEMORY MANAGEMENT -------------------- */
#define OS_MEM_EN                 1    /* Enable (1) or Disable (0) code generation for MEMORY MANAGER */
#define OS_MEM_QUERY_EN           1    /*     Include code for OSMemQuery()                            */
#define OS_MEM_NAME_SIZE         32    /*     Determine the size of a memory partition name            */

#endif
