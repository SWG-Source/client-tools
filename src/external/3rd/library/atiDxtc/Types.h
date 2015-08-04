/******************************************************************************
 *  Types.h -- Variable prototypes
 ******************************************************************************
 $Header:: /Sushi/RunTime/Types.h 12    12/01/00 5:28p Avlachos               $
 ******************************************************************************
 *  Created by Alex Vlachos (AVlachos@ati.com) on 3-26-2001
 *  (C) 2000 ATI Research, Inc.  All rights reserved.
 ******************************************************************************/

#ifndef __Types_h
#define __Types_h

// DEFINES ====================================================================
#ifdef TRUE
#undef TRUE
#endif
#define TRUE  1

#ifdef FALSE
#undef FALSE
#endif
#define FALSE 0

#ifdef ATI_MAC_OS
   /*******/
   /* MAC */
   /*******/
   // SIGNED TYPEDEFS ============================================================
   typedef char char8;
   
   typedef char    int8;
   typedef short   int16;
   typedef long    int32;
   //typedef __int64 int64;
   
   typedef float       float32;
   typedef double      float64;
   typedef long double float80;
   
   typedef long fixed32;
   
   typedef char bool8;
   
   // UNSIGNED TYPEDEFS ==========================================================
   typedef unsigned char    uint8;
   typedef unsigned short   uint16;
   //typedef unsigned long     uint32;
   //typedef unsigned __int64 uint64;
   
   typedef unsigned long ufixed32;
#else   
   /***********/
   /* Windows */
   /***********/
   // SIGNED TYPEDEFS ============================================================
   typedef char char8;
 
  //EAS I commented these out due to conflicts with engine defintions
//   typedef char    int8;
   typedef short   int16;
//   typedef int     int32;
//   typedef __int64 int64;
   
   typedef float       float32;
   typedef double      float64;
   typedef long double float80;
   
   typedef int fixed32;
   
   typedef char bool8;
   
   // UNSIGNED TYPEDEFS ==========================================================
   typedef unsigned char    uint8;
   typedef unsigned short   uint16;
//   typedef unsigned int     uint32;
   typedef unsigned __int64 uint64;
   
   typedef unsigned int ufixed32;
#endif

#endif
