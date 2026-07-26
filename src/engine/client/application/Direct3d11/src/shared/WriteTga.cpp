//
// WriteTGA.cpp
// asommers 11-15-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "FirstDirect3d11.h"
#include "WriteTGA.h"

#include <stdio.h>

//-------------------------------------------------------------------

struct TGAHeader
{
	unsigned char  m_idLength;                              // number of bytes in image ID field
	unsigned char  m_colorMapType;                          // 0 = no color map, 1 = color map
	unsigned char  m_imageType;                             // we only support 2 (uncompressed, RGB)
	unsigned short m_colorMapFirstEntryIndex;               // ignored
	unsigned short m_colorMapLength;                        // ignored
	unsigned char  m_colorMapEntrySize;                     // ignored
	unsigned short m_xOrigin;
	unsigned short m_yOrigin;
	unsigned short m_imageWidth;
	unsigned short m_imageHeight;
	unsigned char  m_pixelDepth;
	unsigned char  m_imageDescriptor;
};

//-------------------------------------------------------------------

void WriteTGA::write (const char* filename, int width, int height, const uint8* data, bool hasAlpha, int pitch)
{
	TGAHeader header;

	char headerBytes[ 18 ];

	::memset( &header, 0, sizeof( header ) );
	::memset( headerBytes, 0, sizeof( headerBytes ) );

	int bytesPerPixel = (hasAlpha ? 4 : 3);
	int iRowSize      = width * bytesPerPixel;
	if (pitch == 0)
		pitch = iRowSize;

	header.m_imageType       = 2;
	header.m_imageDescriptor = hasAlpha ? static_cast<uint8> (8) : static_cast<uint8> (0);
	header.m_imageWidth      = static_cast<uint16> (width);
	header.m_imageHeight     = static_cast<uint16> (height);
	header.m_pixelDepth      = hasAlpha ? static_cast<uint8> (32) : static_cast<uint8> (24);

	headerBytes[ 0 ] = header.m_idLength;
	headerBytes[ 1 ] = header.m_colorMapType;
	headerBytes[ 2 ] = header.m_imageType;
	headerBytes[ 12 ] = static_cast<int8> (static_cast<int8> (header.m_imageWidth) & 0xFF);
	headerBytes[ 13 ] = static_cast<int8> (( header.m_imageWidth >> 8 ) & 0xFF);
	headerBytes[ 14 ] = static_cast<int8> (header.m_imageHeight & 0xFF);
	headerBytes[ 15 ] = static_cast<int8> (( header.m_imageHeight >> 8 ) & 0xFF);
	headerBytes[ 16 ] = header.m_pixelDepth;
	headerBytes[ 17 ] = header.m_imageDescriptor;

	data += (height - 1) * pitch;

	FILE* outfile = fopen (filename, "wb");
	if (!outfile)
		return;

	fwrite (headerBytes, sizeof (headerBytes), 1, outfile);

	for (int i = 0; i < height; i++, data -= pitch)
		fwrite (data, iRowSize, 1, outfile);

	fclose (outfile);
}

//-------------------------------------------------------------------
