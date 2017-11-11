// ======================================================================
//
// FontExporter.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FontExporter_H
#define INCLUDED_FontExporter_H

#include "Unicode.h"
#include <set>

class FontImageExporter;
class CFont;

// ======================================================================
/**
* An abstract interface that represents an exporter that knows how to
* export a set of character codes.
*/

class FontExporter
{
public:

	struct PadInfo
	{
		size_t left;
		size_t right;
		size_t top;
		size_t bottom;
	};

	struct ExporterInfo
	{
		CFont *      cFont;
		size_t       fontSize;
		size_t       tileX;
		size_t       tileY;
		bool         drawOutlines;
		bool         antialias;
		const char * styleName;
		const char * styleFilename;
		const char * imageName;
		const char * imageDir;
		PadInfo      pad;

	};

	typedef std::set<Unicode::unicode_char_t> IdSet_t;


	virtual             ~FontExporter () = 0;

	virtual bool         export (const IdSet_t & idSet) = 0;

protected:
	                     FontExporter (const FontImageExporter & imexp, const ExporterInfo & info);

	const FontImageExporter & getImageExporter () const;
	ExporterInfo         m_info;

private:
	FontExporter (const FontExporter & rhs);
	FontExporter & operator= (const FontExporter & rhs);

	const FontImageExporter &       m_imageExporter;

};

//-----------------------------------------------------------------
inline const FontImageExporter & FontExporter::getImageExporter () const
{
	return m_imageExporter;
}

// ======================================================================

#endif
