// ======================================================================
//
// FileImporterExporter.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FileImporterExporter_H
#define INCLUDED_FileImporterExporter_H

// ======================================================================

class LocalizedStringTableRW;

#include "Unicode.h"

//-----------------------------------------------------------------

class FileImporterExporter
{
public:

	static LocalizedStringTableRW * importAscii   (const char * filename, const Unicode::NarrowString & delimiter, std::string & resultMsg);
	static LocalizedStringTableRW * importUnicode (const char * filename, const Unicode::String & delimiter,       std::string & resultMsg);

	static bool exportAscii   (const char * filename, const Unicode::NarrowString & delimiter, const LocalizedStringTableRW & table, std::string & resultMsg);
	static bool exportUnicode (const char * filename, const Unicode::String & delimiter,       const LocalizedStringTableRW & table, std::string & resultMsg);

private:
	FileImporterExporter  ();
	FileImporterExporter (const FileImporterExporter & rhs);
	FileImporterExporter & operator= (const FileImporterExporter & rhs);
};

// ======================================================================

#endif
