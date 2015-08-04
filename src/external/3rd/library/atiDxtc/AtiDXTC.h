/******************************************************************************
 *  AtiDXTC.h
 ******************************************************************************
 $Header:: /Sushi/RunTime/AtiDXTC.h 4     1/18/01 12:11a Avlachos             $
 ******************************************************************************
 *  Created by Alex Vlachos (AVlachos@ati.com) on 3-26-2001
 *  (C) 2000 ATI Research, Inc.  All rights reserved.
 ******************************************************************************/

#ifndef __AtiDXTC_h
#define __AtiDXTC_h

// INCLUDES ===================================================================
#include "types.h"

// ENUMS ======================================================================
enum //For texture compression routines
{
   SU_RGBA = 0,
   SU_RGB,
   SU_BGRA,
   SU_BGR,
   SU_DXT1,
   SU_DXT3,
   SU_DXT5
};

// FUNCTION DEFS ==============================================================
//=============================================================================
// Parameters:
//   uint8 *original - original 3 or 4 component image
//   int32 width  - image width in pixels
//   int32 height - image height in pixels
//   uint32 format - (SU_RGB | SU_RGBA | SU_BGR | SU_BGRA)
//   uint32 compressedFormat - (SU_DXT1 | SU_DXT3 | SU_DXT5)
//   uint16 *comp - storage for final compressed image
//
// Returns number of bytes added to *comp (-1 for error)
//=============================================================================
int AtiDXTCCompressRGB4 (unsigned char *original, int width, int height, unsigned int format,
                           unsigned int compressedFormat, unsigned short *comp);


uint8* AtiDXTCDecompressMipChain(bool8 aMipMapped, int32 aWidth, int32 aHeight, uint8 *aCompressedTexels, uint8 *aType);
void AtiDXTCFreeMemory(void);
void AtiDXTCDecompressDXT1Block (uint16 *block, uint8 dest[4][4][4], bool8 dxt1);
void AtiDXTCDecompressDXT3Block (uint16 *block, uint8 dest[4][4][4]);
void AtiDXTCDecompressDXT5Block (uint16 *block, uint8 dest[4][4][4]);

#endif
