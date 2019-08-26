/********************************************************************
*System Development Type Define Computer Software
*Copyright (c) 2019-2029, XXX LTD., All rights reserved.
*File Name:   TypeDefine.h
*File Description: Implementation of  TypeDefine.h
*Modification History:
*a	 19-08-2019 15:43:08			WangLin  Created
********************************************************************/

#ifndef _TYPE_DEFINE_H_INCLUDE_
#define _TYPE_DEFINE_H_INCLUDE_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "stm32f767xx.h"



#ifdef __cplusplus
extern "C" {
#endif
#pragma  pack()


typedef bool               BOOL;
typedef unsigned char      UINT8;
typedef unsigned short     UINT16;
typedef unsigned long      UINT32;
typedef unsigned long long UINT64;
typedef signed char        INT8;
typedef signed short int   INT16;
typedef signed long int    INT32;
typedef long long          INT64;
typedef float              FLOAT;
typedef double             DOUBLE;


#ifndef NULL
#define NULL 0
#endif


#define TRUE   (BOOL)1
#define FALSE  (BOOL)0
#define ERROR  ((INT32)(-1))    


#define UINT32_MAX (4294967295U)		/**< UINT32 Max*/
#define UINT16_MAX (65535U)        	/**< UINT16 Max*/
#define UINT8_MAX  (255)          	/**< UINT8 Max*/
#define LONG_MIN   (-2147483647L - 1)
#define LONG_MAX   (2147483647L)



#pragma  pack()
#ifdef __cplusplus
}
#endif

#endif /*!defined(_TYPE_DEFINE_H_INCLUDE_)*/

