// ======================================================================
//
// StringTable.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_StringTable_H
#define INCLUDED_StringTable_H

// ======================================================================

#include "LocalizedString.h"

class StringTable
{
public:

	StringTable ();
	~StringTable ();

	bool load (char const * fileName);

	int getNumberOfStrings () const;
	std::string const & getStringId (int index) const;
	std::string const & getString (int index) const;

	LocalizedString::crc_type const & getCRC(int index) const;
	LocalizedString::crc_type const & getSourceCRC(int index) const;

	int getIndexForStringId (std::string const & stringId);

	int getNumberOfWords () const;
	int getNumberOfWords (int index) const;

	int getNumberOfLines () const;

private:

	typedef std::vector<std::pair<std::string, std::string> > StringList;
	StringList m_stringList;
	
	typedef std::vector<LocalizedString::crc_type> ChecksumList;
	ChecksumList m_crcList;
	ChecksumList m_crcSourceList;

	typedef std::map<std::string, int> StringIdToIndexMap;
	StringIdToIndexMap m_stringIdToIndexMap;
};

// ======================================================================

#endif
