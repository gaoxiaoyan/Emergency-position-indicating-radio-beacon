
#ifndef __DATATYPE_H__
#define __DATATYPE_H__

#ifndef UINT8 
#define UINT8 unsigned char
#endif

#ifndef UINT16
#define UINT16 unsigned int 
#endif

#ifndef UINT32 
#define UINT32 unsigned long
#endif

#ifndef uint8
#define uint8 unsigned char
#endif

#ifndef uint16 
#define uint16 unsigned int
#endif

#ifndef uint32 
#define uint32 unsigned long
#endif

#ifndef SINT8 
#define SINT8 signed char
#endif

#ifndef SINT16 
#define SINT16 signed int
#endif

#ifndef SINT32
#define SINT32 signed long
#endif

#ifndef BOOL
#define BOOL UINT8
#endif

#ifndef bool
#define bool UINT8 
#endif

#ifndef FLOAT32
#define FLOAT32 float
#endif

#define BOOLEAN   BOOL
#define INT8U     UINT8
#define INT8S     SINT8
#define INT16U    UINT16
#define INT16S    SINT16
#define INT32U    UINT32
#define INT32S    SINT32
#define FP32      FLOAT32

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef true
#define true  1
#endif

#ifndef false
#define false 0
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif


#endif