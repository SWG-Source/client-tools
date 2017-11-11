// ======================================================================
//
// StringTable.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_StringTable_H
#define INCLUDED_StringTable_H

// ======================================================================

#include <map>
#include <string>

#include "Unicode.h"

// ======================================================================

class StringTable
{
public:
	
	typedef std::map<std::string, Unicode::String> StringMap;

public:

	explicit StringTable(char const * fileName = 0);
	~StringTable();

	std::string const & getFileName() const;

	int getNumberOfStrings () const;
	std::string const & getKey(int index) const;
	Unicode::String const & getValue(int index) const;
	Unicode::String const & getValue(std::string const & key) const;
	void setString(std::string const & key, Unicode::String const & value);

	bool load(char const * fileName);
	bool save(char const * fileName = 0) const;

private:

	std::string m_fileName;
	StringMap m_stringMap;

private:
	
	StringTable(StringTable const &);
	StringTable & operator=(StringTable const &);
};

// ======================================================================

#endif
