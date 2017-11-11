// ======================================================================
//
// FontImageExporterDDS.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FontImageExporterDDS_H
#define INCLUDED_FontImageExporterDDS_H

#include "FontImageExporter.h"

// ======================================================================
class FontImageExporterDDS : public FontImageExporter
{
public:
		                FontImageExporterDDS ();
	virtual            ~FontImageExporterDDS ();
	virtual bool        exportImage (HDC hdc, HBITMAP hbmp, const char * filename) const;
private:
	FontImageExporterDDS (const FontImageExporterDDS & rhs);
	FontImageExporterDDS & operator= (const FontImageExporterDDS & rhs);
};

// ======================================================================

#endif
