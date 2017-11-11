/**************************************************************************

	File:			order.h

	Description:	handle byte swapping order for TPO Database Server/API

	Copyright 1999 Verant Interactive Inc.

	Confidential and Proprietary to Verant Interactive Inc.

	History:	Comment				Author

	5-21-99		Created				GRI/Fippy

	Sample header created			RSS

**************************************************************************/

#ifndef _ORDER_H_
#define _ORDER_H_

#ifdef __sgi
#define BIG_ENDIAN_HOST
#endif

#ifdef sparc
#define BIG_ENDIAN_HOST
#endif

#ifdef BIG_ENDIAN_HOST
#include "utypes.h"

/* must swap byte order */

#ifdef __cplusplus
extern "C" {
#endif


uint16 htog16( uint16 x );
uint32 htog32( uint32 x );
uint64 htog64( uint64 x );

/* #else */
/* game byte order == host byte order byte order */
/* byte order conversions were defined out in message.h */
#ifdef __cplusplus
}
#endif

#else

#define htog16(x) (x)
#define htog32(x) (x)
#define htog64(x) (x)

#endif

#endif