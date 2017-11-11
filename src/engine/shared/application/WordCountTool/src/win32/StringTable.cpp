// ======================================================================
//
// StringTable.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstWordCountTool.h"
#include "StringTable.h"

#include "fileInterface/StdioFile.h"
#include "LocalizedStringTableReaderWriter.h"
#include "LocalizedString.h"
#include "UnicodeUtils.h"

// ======================================================================

StringTable::StringTable () :
	m_stringList ()
{
}

// ----------------------------------------------------------------------

StringTable::~StringTable ()
{
}

// ----------------------------------------------------------------------

int StringTable::getNumberOfStrings () const
{
	return static_cast<int> (m_stringList.size ());
}

// ----------------------------------------------------------------------

std::string const & StringTable::getStringId (int const index) const
{
	return m_stringList [static_cast<size_t> (index)].first;
}

// ----------------------------------------------------------------------

std::string const & StringTable::getString (int const index) const
{
	return m_stringList [static_cast<size_t> (index)].second;
}

// ----------------------------------------------------------------------

LocalizedString::crc_type const & StringTable::getCRC(int index) const
{
	return m_crcList[index];
}

// ----------------------------------------------------------------------

LocalizedString::crc_type const & StringTable::getSourceCRC(int index) const
{
	return m_crcSourceList[index];
}

// ----------------------------------------------------------------------

int StringTable::getIndexForStringId (std::string const & stringId)
{
	StringIdToIndexMap::iterator fit = m_stringIdToIndexMap.find(stringId);
	if(fit != m_stringIdToIndexMap.end())
		return fit->second;

	return -1;
}

// ----------------------------------------------------------------------

int StringTable::getNumberOfWords () const
{
	int total = 0;
	for (int i = 0; i < getNumberOfStrings (); ++i)
	{
		total += getNumberOfWords(i);
	}

	return total;
}


// ----------------------------------------------------------------------

int StringTable::getNumberOfWords (int index) const
{
	int total = 0;
	char buffer [1024];
	char const * string = getString (index).c_str ();
	while (*string && sscanf (string, "%s", buffer) != 0)
	{
		++total;
		while (*string && (*string != ' ' && *string != '\t' && *string != '\r' && *string != '\n'))
			++string;

		while (*string && (*string == ' ' || *string == '\t' || *string == '\r' || *string == '\n'))
			++string;
	}

	return total;
}

// ----------------------------------------------------------------------

int StringTable::getNumberOfLines () const
{
	int total = 0;

	for (int i = 0; i < getNumberOfStrings (); ++i)
	{
		char const * string = getString (i).c_str ();
		while (*string)
		{
			if (*string == '\n')
				++total;

			++string;
		}

		++total;
	}

	return total;
}

// ----------------------------------------------------------------------

bool StringTable::load (char const * const fileName)
{
	StdioFileFactory fileFactory;
	LocalizedStringTableRW * const stringTable = LocalizedStringTableRW::loadRW (fileFactory, std::string (fileName));
	if (stringTable)
	{
		LocalizedStringTable::Map_t const & stringMap = stringTable->getMap ();
		LocalizedStringTable::Map_t::const_iterator end = stringMap.end ();
		int count = 0;
		for (LocalizedStringTable::Map_t::const_iterator iter = stringMap.begin (); iter != end; ++iter)
		{	
			LocalizedString const * const value = iter->second;
			Unicode::NarrowString const * const stringId = stringTable->getNameById(value->getId());
			Unicode::NarrowString const string = Unicode::wideToNarrow(value->getString());

			LocalizedString::crc_type const crc = LocalizedStringTableRW::str_getCrc(const_cast<LocalizedString &>(*value));
			m_stringList.push_back(std::make_pair(*stringId, string));
			m_crcList.push_back(crc);
			m_stringIdToIndexMap.insert(std::make_pair(*stringId, count));
			count++;
		}

		delete stringTable;

		return true;
	}

	return false;
}

// ======================================================================
