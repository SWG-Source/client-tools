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

	StringTable();
	~StringTable();

	void clear();

	int getNumberOfStrings() const;
	CString const & getStringKey(int index) const;
	CString const & getString(CString const & key) const;
	bool hasStringKey(CString const & key) const;
	void setString(CString const & key, CString const & value);
	void removeString(CString const & key);
	
	bool load(char const * fileName);
	bool save(char const * fileName) const;

private:

	typedef std::map<CString, CString> StringMap;
	StringMap m_stringMap;
};

// ======================================================================

#endif
