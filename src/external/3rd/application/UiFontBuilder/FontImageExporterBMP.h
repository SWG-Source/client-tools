// ======================================================================
//
// FontImageExporterBMP.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FontImageExporterBMP_H
#define INCLUDED_FontImageExporterBMP_H

#include "FontImageExporter.h"

// ======================================================================
class FontImageExporterBMP : public FontImageExporter
{
public:
		                FontImageExporterBMP ();
	virtual            ~FontImageExporterBMP ();
	virtual bool        exportImage (HDC hdc, HBITMAP hbmp, const char * filename) const;

private:
	FontImageExporterBMP (const FontImageExporterBMP & rhs);
	FontImageExporterBMP & operator= (const FontImageExporterBMP & rhs);
};

// ======================================================================

#endif
