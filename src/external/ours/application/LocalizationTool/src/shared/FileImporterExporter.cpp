// ======================================================================
//
// FileImporterExporter.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "StdAfx.h"
#include "FileImporterExporter.h"

#include "LocalizedStringTableReaderWriter.h"
#include "UnicodeUtils.h"

#include <cassert>
#include <cstdio>
#include <windows.h>

// ======================================================================

namespace
{
	//----------------------------------------------------------------------

	const int utf_16_little_endian_BOM = 0xfeff;
	const int utf_16_big_endian_BOM    = 0xfffe;


	//----------------------------------------------------------------------

	LocalizedString * constructLocalizedString (size_t id, const Unicode::String & value)
	{
		return new LocalizedString (id, value);
	}

	//----------------------------------------------------------------------

	LocalizedString * constructLocalizedString (size_t id, const Unicode::String & value, LocalizedString::crc_type crc)
	{
		return new LocalizedString (id, crc, value);
	}

	//----------------------------------------------------------------------

	LocalizedString * constructLocalizedString (size_t id, const Unicode::NarrowString & value)
	{
		return new LocalizedString (id, Unicode::narrowToWide (value));
	}

	//----------------------------------------------------------------------

	LocalizedString * constructLocalizedString (size_t id, const Unicode::NarrowString & value, LocalizedString::crc_type crc)
	{
		return new LocalizedString (id, crc, Unicode::narrowToWide (value));
	}

	//----------------------------------------------------------------------

	template <typename T> T convertString (const Unicode::NarrowString & str)
	{
		return T (str.begin (), str.end ());
	}

	//----------------------------------------------------------------------

	template <typename T> std::string const convertStringToNarrow(T const & string)
	{
		return std::string(string.begin(), string.end());
	}

	//----------------------------------------------------------------------
	
	template <typename T> T * loadFileIntoString (const char * filename, std::string & resultMsg)
	{
		assert (filename);
		
		FILE * fl = fopen (filename, "rb");
		
		if (!fl)
		{
			char msgBuf [1024];
			_snprintf (msgBuf, 1024, "Failed to open file: %s.  Error: %s\n", filename, strerror (errno));
			resultMsg = msgBuf;
			return 0;
		}
		
		fseek (fl, 0, SEEK_END);
		const int len = ftell (fl);
		fseek (fl, 0, SEEK_SET);
		
		size_t const buflen = static_cast<size_t>(len) / sizeof(T::value_type);
		T::value_type * buf = new T::value_type [buflen + 1];
		
		assert (buf);
		
		if (!fread (buf, buflen, sizeof (T::value_type), fl))
		{
			char msgBuf [1024];
			_snprintf (msgBuf, 1024, "Failed to read %d %d byte elements from file: %s.  Error: %s\n", buflen, sizeof (T::value_type), filename, strerror (errno));
			resultMsg = msgBuf;
			fclose (fl);

			delete[] buf;
			buf = 0;

			return 0;
		}
		
		fclose (fl);
		
		buf [buflen] = 0;
		T * strbuf = new T (buf);
		delete[] buf;
		buf = 0;

		//-----------------------------------------------------------------
		//-- delete those fucking \r characters that windows apps pollute text files with
		{
			size_t crpos = 0;
			while ((crpos = strbuf->find ('\r')) != strbuf->npos)
				strbuf->erase (crpos, 1);
		}

		return strbuf;
	}
	
	//----------------------------------------------------------------------
	
	template <typename T> LocalizedStringTableRW * importAny   (const T & strbuf, const T & delimiter, std::string & resultMsg)
	{
		//-----------------------------------------------------------------
		//-- create the table and start populating it
		
		LocalizedStringTableRW * const table = new LocalizedStringTableRW ("bloohahaha");
		assert (table);
		
		size_t pos = 0;
		size_t endpos = 0;
		
		T const desiredVersionNumber(convertString<T>(std::string("v0001")));
		T versionNumber;
		T token_entry_key;
		T tokenCrc;
		const T newline (1, '\n');
		const T modified_delimiter = newline + delimiter;

		unsigned long id = 0;

		//-- Read version number
		if (!Unicode::getFirstToken(strbuf, pos, endpos, versionNumber, newline.c_str()) || versionNumber != desiredVersionNumber)
		{
			resultMsg = "Import failed.  Missing version header.  Please reexport original file using the latest version of the LocalizationTool.\n";
			delete table;
			return 0;
		}
		
		pos = endpos;

		for (;;)
		{
			if (!Unicode::getFirstToken (strbuf, pos, endpos, token_entry_key, newline.c_str ()))
				break;

			pos = endpos;

			if (!Unicode::getFirstToken(strbuf, pos, endpos, tokenCrc, newline.c_str()))
				break;

			pos = endpos;

			LocalizedString * locstr = 0;
			size_t endOfData = static_cast<size_t>(strbuf.npos);
			
			if (pos == strbuf.npos)
			{
				locstr = new LocalizedString (++id, 0, Unicode::emptyString);
			}
			else
			{	
				endOfData              = strbuf.find (modified_delimiter, pos);
				assert (endOfData > pos);

				//-- skip the \n at pos
				const T token_entry_value = strbuf.substr (pos + 1, endOfData == strbuf.npos ? strbuf.npos : endOfData - pos - 1);
				locstr      = constructLocalizedString (++id, token_entry_value);				
			}
			
			std::string const narrowCrc(convertStringToNarrow<T>(tokenCrc));
			locstr->setSourceCrc(atoi(narrowCrc.c_str()));

			LocalizedString * const current = table->getLocalizedStringByName (token_entry_key);
			
			if (current)
			{
				char buf [2048];
				_snprintf (buf, sizeof (buf),
					"Import failed.\n"
					"Duplicate entries detected.\n"
					"Attempted to import entry     id=[%3d] name=[%s] str=[%s]\n"
					"Conflicts with previous entry str=[%s]\n",

					id, 
					token_entry_key.c_str (), 
					Unicode::wideToNarrow (locstr->getString ()).c_str (),
					Unicode::wideToNarrow (current->getString ()).c_str ());
				resultMsg += buf;

				delete table;
				return 0;
			}
			else
			{
				LocalizedString * const result  = table->addString (locstr, token_entry_key, resultMsg);
				assert (result);
				LOC_UNREF (result);
			}
			
			if (endOfData == strbuf.npos)
				break;
			else 
				pos = strbuf.find ('\n', endOfData + delimiter.size ());
		}
		
		return table;
	}
	
}

//----------------------------------------------------------------------

LocalizedStringTableRW * FileImporterExporter::importAscii   (const char * filename, const Unicode::NarrowString & delimiter, std::string & resultMsg)
{
	Unicode::NarrowString * const strbuf = loadFileIntoString<Unicode::NarrowString>(filename, resultMsg);
	if (!strbuf)
		return 0;

	LocalizedStringTableRW * const table = importAny (*strbuf, delimiter, resultMsg);
	delete strbuf;
	return table;
}

//----------------------------------------------------------------------

LocalizedStringTableRW * FileImporterExporter::importUnicode   (const char * filename, const Unicode::String & delimiter, std::string & resultMsg)
{
	Unicode::String * const strbuf       = loadFileIntoString<Unicode::String>(filename, resultMsg);
	if (!strbuf)
		return 0;

	const Unicode::unicode_char_t BOM = (*strbuf) [0];
	if (BOM != utf_16_little_endian_BOM)
	{
		resultMsg = "The specified file does not contain valid little-endian UTF-16 Unicode data.";

		if (BOM == utf_16_big_endian_BOM)
			resultMsg += "The specified file contains big-endian UTF-16 Unicode data (unsupported).";

		return 0;
	}

	strbuf->erase (0, 1);

	LocalizedStringTableRW * const table =  importAny (*strbuf, delimiter, resultMsg);
	delete strbuf;
	return table;
}

//----------------------------------------------------------------------

namespace
{
	
	template <typename T> bool exportAny (FILE * fl, const T & delimiter, const LocalizedStringTableRW & table)
	{
		{
			//-- write the version number at the top
			std::string const narrowVersionNumber("v0001\n");
			T const convertedVersionNumber = convertString<T>(narrowVersionNumber);
			fwrite(convertedVersionNumber.c_str(), sizeof(T::value_type), convertedVersionNumber.size(), fl);
		}

		const LocalizedStringTable::Map_t & tmap = static_cast<const LocalizedStringTable &>(table).getMap ();
		T::value_type const newline = '\n';

		for (LocalizedStringTable::Map_t::const_iterator it = tmap.begin (); it != tmap.end (); ++it)
		{
			const size_t id                = (*it).first;
			LocalizedString * const locstr = (*it).second;
			
			const Unicode::NarrowString * const name = table.getNameById (id);
			const T converted_name = convertString<T> (*name);
			
			fwrite (converted_name.c_str (),       sizeof (T::value_type), converted_name.size (), fl);
			fwrite (&newline,                      sizeof (T::value_type), 1,                      fl);

			char buffer[16];
			sprintf(buffer, "%i", locstr->getCrc());
			std::string const narrowCrc(buffer);
			T const convertedCrc = convertString<T>(narrowCrc);

			fwrite(convertedCrc.c_str(), sizeof(T::value_type), convertedCrc.size(), fl);
			fwrite(&newline, sizeof(T::value_type), 1, fl);
			
			const Unicode::String & str            = locstr->getString ();
			const T convertedString (str.begin (), str.end ());

			fwrite (convertedString.c_str (),      sizeof (T::value_type), str.size (),  fl);
			if (str [str.size () - 1] != '\n')
				fwrite (&newline,                  sizeof (T::value_type), 1,                      fl);
			fwrite (delimiter.c_str (),            sizeof (T::value_type), delimiter.size (),      fl);
			fwrite (&newline,                      sizeof (T::value_type), 1,                      fl);
		}

		fclose (fl);

		return true;
		
	}

	//----------------------------------------------------------------------

	FILE * openExportFile (const char * filename, std::string & resultMsg)
	{
		assert (filename);
		
		FILE * const fl = fopen (filename, "wb");
		
		if (!fl)
		{
			char msgBuf [1024];
			_snprintf (msgBuf, 1024, "Failed to open file: %s.  Error: %s\n", filename, strerror (errno));
			resultMsg = msgBuf;
			return 0;
		}

		return fl;
	}
}

//----------------------------------------------------------------------

bool FileImporterExporter::exportUnicode (const char * filename, 
										const Unicode::String & delimiter,
										const LocalizedStringTableRW & table,
										std::string & resultMsg)
{
	
	FILE * const fl = openExportFile (filename, resultMsg);
	
	if (!fl)
		return 0;
	
	//-- write the BOM (byte order marker)
	fwrite (&utf_16_little_endian_BOM, sizeof (Unicode::unicode_char_t), 1, fl);
	const bool retval = exportAny (fl, delimiter, table);
	fclose (fl);
	return retval;
}

//----------------------------------------------------------------------

bool FileImporterExporter::exportAscii   (const char * filename,
										const Unicode::NarrowString & delimiter,
										const LocalizedStringTableRW & table,
										std::string & resultMsg)
{
	FILE * const fl = openExportFile (filename, resultMsg);
	
	if (!fl)
		return 0;
	
	const bool retval = exportAny (fl, delimiter, table);
	fclose (fl);
	return retval;
}

// ======================================================================
