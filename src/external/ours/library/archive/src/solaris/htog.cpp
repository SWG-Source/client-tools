/**************************************************************************

	File:			order.cpp

	Description:	handle byte swapping order for TPO Database Server/API

	Copyright 1999 Verant Interactive Inc.

	Confidential and Proprietary to Verant Interactive Inc.

	History:	Comment				Author

	5-21-99		Created				GRI/Fippy

	Sample header created			RSS

**************************************************************************/

#include "htog.h"

uint16 htog16( uint16 x )
{
  union
  {
    uint16 src;
    uint8  srca[2];
  }src;
  union
  {
    uint16 dst;
    uint8  dsta[2];
  }dst;
  src.src = x;
  dst.dsta[0] = src.srca[1];
  dst.dsta[1] = src.srca[0];
  return dst.dst;
}

uint32 htog32( uint32 x )
{
  union
  {
    uint32 src;
    uint8  srca[4];
  } src;
  union
  {
    uint32 dst;
    uint8  dsta[4];
  } dst;
  src.src = x;
  dst.dsta[0] = src.srca[3];
  dst.dsta[1] = src.srca[2];
  dst.dsta[2] = src.srca[1];
  dst.dsta[3] = src.srca[0];
  return dst.dst;
}

uint64 htog64( uint64 x )
{
  union
  {
    uint64 src;
    uint8  srca[8];
  }src;
  union
  {
    uint64 dst;
    uint8  dsta[8];
  }dst;
  src.src = x;
  dst.dsta[0] = src.srca[7];
  dst.dsta[1] = src.srca[6];
  dst.dsta[2] = src.srca[5];
  dst.dsta[3] = src.srca[4];
  dst.dsta[4] = src.srca[3];
  dst.dsta[5] = src.srca[2];
  dst.dsta[6] = src.srca[1];
  dst.dsta[7] = src.srca[0];
  return dst.dst;
}
