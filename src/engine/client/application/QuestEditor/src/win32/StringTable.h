// ======================================================================
//
// StringTable.h
// Copyright 2005, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_StringTable_H
#define INCLUDED_StringTable_H

// ----------------------------------------------------------------------

#include <qstring.h>
#include <map>

// ======================================================================

class StringTable
{
public:

	StringTable();
	~StringTable();

	void clear();

	int getNumberOfStrings() const;
	QString const & getStringKey(int index) const;
	QString const & getString(QString const & key) const;
	bool hasStringKey(QString const & key) const;
	void setString(QString const & key, QString const & value);
	void removeString(QString const & key);
	
	bool load(char const * fileName);
	bool save(char const * fileName) const;

private:

	typedef std::map<QString, QString> StringMap;
	StringMap m_stringMap;
};

// ======================================================================

#endif
