// ======================================================================
// TextureFormatInfo.h
//
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TextureFormatInfo_H
#define INCLUDED_TextureFormatInfo_H

// ======================================================================

#include "clientGraphics/Texture.def"

// ======================================================================

struct TextureFormatInfo
{
	bool        supported;
  bool        compressed;

	int         pixelByteCount;
	int         aBitCount;
  int         rBitCount;
	int         gBitCount;
	int         bBitCount;

	uint32      aMask;
	uint32      rMask;
	uint32      gMask;
	uint32      bMask;

	// Information about the DXTC compression scheme
	int         blockWidth;
	int         blockHeight;
	int         blockSize;

	char const *name;

	DLLEXPORT static const TextureFormatInfo &getInfo(TextureFormat format);
	DLLEXPORT static void                     setSupported(TextureFormat format, bool supported);
};

// ======================================================================

#endif
