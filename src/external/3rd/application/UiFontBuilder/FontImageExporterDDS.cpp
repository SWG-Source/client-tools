// ======================================================================
//
// FontImageExporterDDS.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "StdAfx.h"
#include "FontImageExporterDDS.h"
#include <cassert>
#include <string>
#include <ddraw.h>

#undef max
#undef min

// ======================================================================

// ======================================================================
FontImageExporterDDS::FontImageExporterDDS () : FontImageExporter ()
{
	
}
//-----------------------------------------------------------------

FontImageExporterDDS::~FontImageExporterDDS ()
{
	
}
//-----------------------------------------------------------------


bool    FontImageExporterDDS::exportImage (HDC hdc, HBITMAP hbmp, const char * filename) const
{

	assert (filename);

	std::string actualFilename = filename;
	actualFilename += ".dds";

	BITMAPINFO        bmi;
	BITMAP            bm;

	GetObject( hbmp, sizeof( bm ), &bm );

	ZeroMemory( &bmi, sizeof( bmi ) );
	bmi.bmiHeader.biSize        =  sizeof( bmi.bmiHeader );
	bmi.bmiHeader.biWidth       =  bm.bmWidth;
	bmi.bmiHeader.biHeight      =  bm.bmHeight;
	bmi.bmiHeader.biPlanes      =  1;
	bmi.bmiHeader.biBitCount    =  24;
	bmi.bmiHeader.biCompression =  BI_RGB;

	int bmPitch = bm.bmWidth * 3;

	size_t datasize = bm.bmWidth * bm.bmHeight * 3;

	unsigned char *mem = new unsigned char[datasize];
	GetDIBits( hdc, hbmp, 0, bm.bmHeight, mem, &bmi, DIB_RGB_COLORS );

	size_t argb_datasize = bm.bmWidth * bm.bmHeight * 4;
	unsigned char *argb = new unsigned char [argb_datasize];

	unsigned char *argb_ptr = argb;

	// we must flip the image vertically to write the DDS
	for (int row = bm.bmHeight - 1; row >= 0; --row)
	{
		unsigned char *srcPtr = mem + row * bmPitch;

		for (int i = 0; i < bmPitch; i += 3)
		{
			// order: BGRA 
			*(argb_ptr++) = 0xff;
			*(argb_ptr++) = 0xff;
			*(argb_ptr++) = 0xff;

			unsigned char value = static_cast<unsigned char>((srcPtr [i] + srcPtr [i+1] + srcPtr [i+2]) / 3);
			*(argb_ptr++) = value;
		}
	}

	argb_ptr = 0;
	delete[] mem;
	mem = 0;

	FILE *fp = fopen( actualFilename.c_str (), "wb" );

	if (fp == 0)
		return false;

	const DWORD ddsMagic = 0x20534444;

	DDPIXELFORMAT DDSPF_A8R8G8B8 = 
	{
		0,
		DDPF_RGB | DDPF_ALPHAPIXELS,
		0,
		32,
		0x00ff0000,
		0x0000ff00,
		0x000000ff,
		0xff000000
	};

	DDSPF_A8R8G8B8.dwSize = sizeof (DDSPF_A8R8G8B8);

	DDSURFACEDESC2 ddsd = {0};

	ddsd.dwSize   = sizeof (ddsd);
	ddsd.dwFlags  = DDSD_WIDTH | DDSD_HEIGHT |DDSD_PITCH |DDSD_CAPS | DDSD_PIXELFORMAT |DDSD_PITCH;
	ddsd.dwHeight = bm.bmWidth;
	ddsd.dwWidth  = bm.bmHeight;
	ddsd.lPitch   = bm.bmWidth * 4;
	ddsd.dwBackBufferCount = 0;
	ddsd.dwMipMapCount = 1;
	ddsd.dwAlphaBitDepth = 8;
	ddsd.lpSurface = 0;
	ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
	ddsd.ddpfPixelFormat = DDSPF_A8R8G8B8;

	fwrite (&ddsMagic, sizeof (DWORD), 1, fp);
	fwrite (&ddsd,     ddsd.dwSize,    1, fp);

	fwrite (argb,     argb_datasize,  1, fp);
	
	fclose( fp );

	delete[] argb;

	return true;
}

// ======================================================================
