// ======================================================================
//
// StringTable.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstStringFileTool.h"
#include "StringTable.h"

#include "fileInterface/StdioFile.h"
#include "LocalizedStringTableReaderWriter.h"
#include "LocalizedString.h"
#include "UnicodeUtils.h"

#include <vector>

// ======================================================================

StringTable::StringTable(char const * fileName) :
	m_fileName(fileName ? fileName : ""),
	m_stringMap()
{
}

// ----------------------------------------------------------------------

StringTable::~StringTable()
{
}

// ----------------------------------------------------------------------

std::string const & StringTable::getFileName() const
{
	return m_fileName;
}

// ----------------------------------------------------------------------

int StringTable::getNumberOfStrings() const
{
	return m_stringMap.size();
}

// ----------------------------------------------------------------------

std::string const & StringTable::getKey(int index) const
{
	StringMap::const_iterator iterator = m_stringMap.begin();
	for (int i = 0; i < index; ++i)
		++iterator;

	return iterator->first;
}

// ----------------------------------------------------------------------

Unicode::String const & StringTable::getValue(int index) const
{
	StringMap::const_iterator iterator = m_stringMap.begin();
	for (int i = 0; i < index; ++i)
		++iterator;

	return iterator->second;
}

// ----------------------------------------------------------------------

Unicode::String const & StringTable::getValue(std::string const & key) const
{
	return const_cast<StringTable &>(*this).m_stringMap[key];
}

// ----------------------------------------------------------------------

void StringTable::setString(std::string const & key, Unicode::String const & value)
{
	m_stringMap[key] = value;
}

// ----------------------------------------------------------------------

bool StringTable::load(char const * const fileName)
{
	StdioFileFactory fileFactory;
	LocalizedStringTableRW * stringTable = LocalizedStringTableRW::loadRW(fileFactory, std::string(fileName));
	if (stringTable)
	{
		m_fileName = fileName;

		const LocalizedStringTable::Map_t & stringMap = stringTable->getMap();
		LocalizedStringTable::Map_t::const_iterator end = stringMap.end();
		for (LocalizedStringTable::Map_t::const_iterator iter = stringMap.begin(); iter != end; ++iter)
		{	
			LocalizedString const * const value = iter->second;
			unsigned long const stringId = value->getId();
			Unicode::NarrowString const * const key = stringTable->getNameById(stringId);

			m_stringMap[*key] = value->getString();
		}

		delete stringTable;

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

bool StringTable::save(char const * const fileName) const
{
	LocalizedStringTableRW stringTable("temp");

	{
		int id = 0;

		StringMap::const_iterator end = m_stringMap.end();
		for (StringMap::const_iterator iter = m_stringMap.begin(); iter != end; ++iter)
		{
			std::string result;
			stringTable.addString(new LocalizedString(++id, iter->second), iter->first, result);
		}
	}

	StdioFileFactory fileFactory;
	return stringTable.writeRW(fileFactory, fileName ? std::string(fileName) : m_fileName);
}

// ======================================================================
