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

/* must swap byte order */

typedef signed   char 		int8;
typedef unsigned char 		uint8;
typedef signed   short 		int16;
typedef unsigned short 		uint16;
typedef signed   int 		int32;
typedef unsigned int 		uint32;
typedef signed   long long 	int64;
typedef unsigned long long 	uint64;

uint16 htog16( uint16 x );
uint32 htog32( uint32 x );
uint64 htog64( uint64 x );

#endif
