// ======================================================================
//
// MayaCompoundString.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef MAYA_COMPOUND_STRING_H
#define MAYA_COMPOUND_STRING_H

// ======================================================================

#include "maya/MString.h"
#include "maya/MStringArray.h"

// ======================================================================

class MayaCompoundString
{
public:

	MayaCompoundString();
	explicit MayaCompoundString(const MString &compoundString, char separator = '_');
	~MayaCompoundString();

	void           setCompoundString(const MString &compoundString);
	const MString &getCompoundString() const;

	int            getComponentCount() const;
	const MString &getComponentString(int index) const;

private:

	void  buildComponentStrings();

private:

	MString       m_compoundString;
	MStringArray  m_componentStrings;
	char          m_separator;

private:

	// disabled
	MayaCompoundString(const MayaCompoundString&);
	MayaCompoundString &operator =(const MayaCompoundString&);

};

// ======================================================================

inline const MString &MayaCompoundString::getCompoundString() const
{
	return m_compoundString;
}

// ----------------------------------------------------------------------

inline int MayaCompoundString::getComponentCount() const
{
	return static_cast<int>(m_componentStrings.length());
}

// ----------------------------------------------------------------------

inline const MString &MayaCompoundString::getComponentString(int index) const
{
	//-- What am I doing, you ask?  I'm dealing with the fact that Maya's API
	//   writer came up with the inefficient idea to have the const version of
	//   MStringArray[] return a non-const value rather than a const reference.
	//   Go figure.

	return const_cast<MayaCompoundString*>(this)->m_componentStrings[static_cast<size_t>(index)];
}

// ======================================================================

#endif
