// ======================================================================
//
// FontImageExporter.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FontImageExporter_H
#define INCLUDED_FontImageExporter_H

class CBitmap;

#include <afxwin.h>

// ======================================================================
/**
* An abstract interface	that represents an exporter that knows how to
* convert a CBitmap to an specified image file on disk.
*/

class FontImageExporter
{
public:

	virtual            ~FontImageExporter () = 0;
	virtual bool        exportImage (HDC hdc, HBITMAP hbmp, const char * filename) const = 0;

protected:
	FontImageExporter ();
private:

	FontImageExporter (const FontImageExporter & rhs);
	FontImageExporter & operator= (const FontImageExporter & rhs);
};

// ======================================================================

#endif
