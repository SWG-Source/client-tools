// ======================================================================
//
// StringTable.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgConversationEditor.h"
#include "StringTable.h"

#include "fileInterface/StdioFile.h"
#include "LocalizedStringTableReaderWriter.h"
#include "LocalizedString.h"
#include "UnicodeUtils.h"

// ======================================================================

StringTable::StringTable () :
	m_stringMap ()
{
}

// ----------------------------------------------------------------------

StringTable::~StringTable ()
{
}

// ----------------------------------------------------------------------

void StringTable::clear ()
{
	m_stringMap.clear ();
}

// ----------------------------------------------------------------------

std::string const & StringTable::getString (std::string const & key) const
{
	return const_cast<StringTable &> (*this).m_stringMap [key];
}

// ----------------------------------------------------------------------

void StringTable::setString (std::string const & key, std::string const & value)
{
	m_stringMap [key] = value;
}

// ----------------------------------------------------------------------

bool StringTable::load (char const * const fileName)
{
	StdioFileFactory fileFactory;
	LocalizedStringTableRW * stringTable = LocalizedStringTableRW::loadRW (fileFactory, std::string (fileName));
	if (stringTable)
	{
		const LocalizedStringTable::Map_t & stringMap = stringTable->getMap ();
		LocalizedStringTable::Map_t::const_iterator end = stringMap.end ();
		for (LocalizedStringTable::Map_t::const_iterator iter = stringMap.begin (); iter != end; ++iter)
		{	
			LocalizedString const * const value = iter->second;
			unsigned long const stringId = value->getId ();
			Unicode::NarrowString const * const key = stringTable->getNameById (stringId);

			setString (*key, Unicode::wideToNarrow (value->getString ()));
		}

		delete stringTable;

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

bool StringTable::save (char const * const fileName) const
{
	LocalizedStringTableRW stringTable ("temp");

	{
		typedef std::vector<std::pair<std::string, std::string> > StringList;
		StringList stringList;

		{
			StringMap::const_iterator end = m_stringMap.end ();
			for (StringMap::const_iterator iter = m_stringMap.begin (); iter != end; ++iter)
				stringList.push_back (std::make_pair (iter->first, iter->second));
		}

		{
			int id = 0;

			StringList::const_iterator end = stringList.end ();
			for (StringList::const_iterator iter = stringList.begin (); iter != end; ++iter)
			{
				std::string result;
				stringTable.addString (new LocalizedString (++id, Unicode::narrowToWide (iter->second)), iter->first, result);
			}
		}
	}

	StdioFileFactory fileFactory;
	return stringTable.writeRW (fileFactory, std::string (fileName));
}

// ======================================================================
