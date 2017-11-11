// ======================================================================
//
// FontExporterPSUI.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "StdAfx.h"
#include "FontExporterPSUI.h"
#include "FontImageExporter.h"
#include <afxwin.h>
#include <cmath>
#include "FontGlyphCode.h"

#undef max
#undef min

// ======================================================================
FontExporterPSUI::FontExporterPSUI (const FontImageExporter & imexp, const ExporterInfo & info) :
FontExporter (imexp, info)
{
}

//-----------------------------------------------------------------

FontExporterPSUI::~FontExporterPSUI ()
{
}

//-----------------------------------------------------------------

bool FontExporterPSUI::export (const IdSet_t & idSet)
{
	HDC     ScreenDC;
	HDC		RenderDC;		
	HBITMAP	RenderBitmap;
	HBITMAP OldBitmap;
	HFONT   TheFont;
	FILE   *fpStyle;
	int     TileNumber = 0;
	
	std::string dirStr = m_info.imageDir;
	if (dirStr.length ())
		dirStr += "\\";

	ScreenDC     = GetDC( NULL );
	RenderDC     = CreateCompatibleDC( ScreenDC );
	RenderBitmap = CreateCompatibleBitmap( ScreenDC, m_info.tileX, m_info.tileY);
	
	OldBitmap = (HBITMAP)SelectObject( RenderDC, RenderBitmap );
	
	fpStyle = fopen( m_info.styleFilename, "w" );
	
	if( !fpStyle )
	{
		MessageBox( 0, _T("Could not create style file"), 0, MB_OK );
		return false;
	}
	
	fprintf( fpStyle, "<textstyle name='%s' leading='%d'>\n", m_info.styleName, m_info.fontSize);
	
	char CurrentFileShortName[256];
	_snprintf( CurrentFileShortName, 255, "%s_%03d", m_info.imageName, TileNumber );
	char CurrentFilePathName[_MAX_PATH];
	_snprintf( CurrentFilePathName, _MAX_PATH -1, "%s%s", dirStr.c_str (), CurrentFileShortName );

	SelectObject( RenderDC, GetStockObject(NULL_PEN) );
	SelectObject( RenderDC, GetStockObject(BLACK_BRUSH) );
	Rectangle( RenderDC, 0, 0, m_info.tileX, m_info.tileY );
	
	SetTextColor( RenderDC, RGB( 255, 255, 255 ) );
	SetBkMode( RenderDC, TRANSPARENT );
		
	LOGFONT lfont;

	m_info.cFont->GetLogFont (&lfont);

	TheFont = CreateFont(
		m_info.fontSize,
		0, 
		0, 
		0, 
		lfont.lfWeight,
		0,
		0,
		0,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, 	
		CLIP_DEFAULT_PRECIS,
		m_info.antialias ? (PROOF_QUALITY) : NONANTIALIASED_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		lfont.lfFaceName );

	if( !TheFont )
	{
		MessageBox( 0, _T("GDI could not create the requested font"), 0, MB_OK );
		return false;
	}
	
	HFONT OldFont = (HFONT)SelectObject( RenderDC, TheFont );
	
	POINT p = {0, m_info.pad.top};
	
	if( m_info.drawOutlines )
	{
		m_info.pad.left   += 1;
		m_info.pad.right  += 1;
		m_info.pad.top    += 1;
		m_info.pad.bottom += 1;
	}
	
	for (IdSet_t::const_iterator iter = idSet.begin (); iter != idSet.end (); ++iter)
	{
		const Unicode::unicode_char_t i = *iter;
		
		USHORT indx = GetTTUnicodeGlyphIndex(RenderDC, i);
		if(indx == 0)
			continue;

		ABC abc;
		GetCharABCWidths (RenderDC, i, i, &abc);

		const long advancePre = abc.abcA;
				
		const long width   = abc.abcB;
		const long advance = abc.abcA + abc.abcB + abc.abcC;
				
		if( (p.x + m_info.pad.left + width + 1 + m_info.pad.right) > m_info.tileX )
		{
			p.x = 0;
			
			p.y += m_info.pad.top + m_info.fontSize + m_info.pad.bottom;
			
			if( (p.y + m_info.pad.top + m_info.fontSize + m_info.pad.bottom) >  m_info.tileY )
			{
				if (getImageExporter ().exportImage (RenderDC, RenderBitmap, CurrentFilePathName) == false)
				{
					SelectObject (RenderDC, OldBitmap);
					SelectObject (RenderDC, OldFont);
					DeleteObject (RenderBitmap);
					DeleteObject (TheFont);
					DeleteDC     (RenderDC);
					ReleaseDC    (NULL, ScreenDC);
					fclose       (fpStyle);

					MessageBox (0, _T("Error creating a tile.\n"), 0, MB_OK );
				
					return false;
				}
				
				_snprintf( CurrentFileShortName, 255, "%s_%03d", m_info.imageName, ++TileNumber );
				_snprintf( CurrentFilePathName, _MAX_PATH -1, "%s%s", dirStr.c_str (), CurrentFileShortName );

				SelectObject( RenderDC, GetStockObject(NULL_PEN) );
				SelectObject( RenderDC, GetStockObject(BLACK_BRUSH) );
				
				Rectangle( RenderDC, 0, 0, m_info.tileX, m_info.tileY );
				
				p.y = 0;
			}
		}
		
		p.x += m_info.pad.left;

		TCHAR ch = i;
		TextOut( RenderDC, p.x - advancePre, p.y, &ch, 1 );
		
		if( m_info.drawOutlines )
		{
			SelectObject( RenderDC, GetStockObject(WHITE_PEN) );
			SelectObject( RenderDC, GetStockObject(NULL_BRUSH) );
			Rectangle(
				RenderDC, 
				p.x,
				p.y,
				p.x + width + 1,
				p.y + m_info.fontSize + 1);
		}
		
		fprintf( fpStyle, "<fontcharacter name=%04x code=%d advancePre=%d advance=%d sourcefile='font/%s' sourcerect='%d,%d,%d,%d'/>\n",
			i, i, 
			advancePre,
			advance, 
			CurrentFileShortName,
			p.x,
			p.y,
			p.x + width,
			p.y + m_info.fontSize);
		
		p.x += width + 1 + m_info.pad.right;
	}


	fprintf( fpStyle, "</textstyle>\n" );
	fclose( fpStyle );
	
	bool retval = getImageExporter ().exportImage (RenderDC, RenderBitmap, CurrentFilePathName);

	SelectObject( RenderDC, OldBitmap );
	SelectObject( RenderDC, OldFont );
	DeleteObject( RenderBitmap );
	DeleteObject( TheFont );
	DeleteDC( RenderDC );
	ReleaseDC( NULL, ScreenDC );

	if (retval == false)
	{
		MessageBox (0, _T("Error creating last tile.\n"), 0, MB_OK );
	}

	return retval;
}

// ======================================================================