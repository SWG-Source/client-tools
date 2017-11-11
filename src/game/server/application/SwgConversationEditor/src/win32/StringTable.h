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

class StringTable
{
public:

	StringTable ();
	~StringTable ();

	void clear ();

	std::string const & getString (std::string const & key) const;
	void setString (std::string const & key, std::string const & value);

	bool load (char const * fileName);
	bool save (char const * fileName) const;

private:

	typedef std::map<std::string, std::string> StringMap;
	StringMap m_stringMap;
};

// ======================================================================

#endif
