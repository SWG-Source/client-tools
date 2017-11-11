// ======================================================================
//
// FontExporterPSUI.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FontExporterPSUI_H
#define INCLUDED_FontExporterPSUI_H

#include "FontExporter.h"

// ======================================================================
class FontExporterPSUI : public FontExporter
{
public:

	virtual             ~FontExporterPSUI ();

	virtual bool         export (const IdSet_t & idSet);
 
	                     FontExporterPSUI (const FontImageExporter & imexp, const ExporterInfo & info);
private:
	FontExporterPSUI (const FontExporterPSUI & rhs);
	FontExporterPSUI & operator= (const FontExporterPSUI & rhs);
};

// ======================================================================

#endif
