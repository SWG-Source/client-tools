// ======================================================================
//
// FontImageExporterBMP.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "StdAfx.h"
#include "FontImageExporterBMP.h"

#include <string>
#include <cassert>

// ======================================================================
FontImageExporterBMP::FontImageExporterBMP () : FontImageExporter ()
{
	
}
//-----------------------------------------------------------------

FontImageExporterBMP::~FontImageExporterBMP ()
{
	
}
//-----------------------------------------------------------------

bool        FontImageExporterBMP::exportImage (HDC hdc, HBITMAP hbmp, const char * filename) const
{

	assert (filename);

	std::string actualFilename = filename;
	actualFilename += ".bmp";

	BITMAPFILEHEADER  Header;
	BITMAPINFO        bmi;
	BITMAP            bm;
	long              datasize;

	GetObject( hbmp, sizeof( bm ), &bm );

	ZeroMemory( &bmi, sizeof( bmi ) );
	bmi.bmiHeader.biSize        =  sizeof( bmi.bmiHeader );
	bmi.bmiHeader.biWidth       =  bm.bmWidth;
	bmi.bmiHeader.biHeight      =  bm.bmHeight;
	bmi.bmiHeader.biPlanes      =  1;
	bmi.bmiHeader.biBitCount    =  24;
	bmi.bmiHeader.biCompression =  BI_RGB;	

	datasize = bm.bmWidth * bm.bmHeight * 3;

	char *mem = new char[datasize];
	GetDIBits( hdc, hbmp, 0, bm.bmHeight, mem, &bmi, DIB_RGB_COLORS );

	ZeroMemory( &Header, sizeof( Header ) );
	Header.bfType        = 'MB';          // BM reversed due to byte ordering
	Header.bfSize        = sizeof( Header ) + sizeof( bmi ) + datasize;
	Header.bfOffBits = sizeof( Header ) + sizeof( bmi );

	FILE *fp = fopen( actualFilename.c_str (), "wb" );

	if (fp == 0)
		return false;

	fwrite( &Header, sizeof( Header ), 1, fp );
	fwrite( &bmi, sizeof( bmi ), 1, fp );
	fwrite( mem, datasize, 1, fp );
	fclose( fp );

	delete mem;

	return true;
}

// ======================================================================
