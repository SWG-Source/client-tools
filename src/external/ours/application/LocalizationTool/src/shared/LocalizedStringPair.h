// ======================================================================
//
// LocalizedStringPair.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LocalizedStringPair_H
#define INCLUDED_LocalizedStringPair_H

#include "Unicode.h"

#include "LocalizedString.h"

// ======================================================================
class LocalizedStringPair
{
public:
	                                 LocalizedStringPair(const Unicode::NarrowString & name, const LocalizedString & srcString, const LocalizedString * translatedString);
	                                 LocalizedStringPair(const LocalizedStringPair & rhs);
	LocalizedStringPair &            operator=   (const LocalizedStringPair & rhs);

	LocalizedString::id_type         getId() const;
	const Unicode::NarrowString &    getName() const;
	void                             setName(const Unicode::NarrowString & name);

	LocalizedString &                getSourceString();
	LocalizedString &                getTranslatedString();

	const LocalizedString &          getSourceString() const;
	const LocalizedString &          getTranslatedString() const;

	bool                             getHasTranslatedString() const;
	                                 LocalizedStringPair();

	bool                             isOutOfDate() const;

	void                             setHasTranslatedString(bool b);

	bool                             isModified(int index) const;
	void                             setModified(int index, bool b);

	void updateCrc();

private:

private:

	Unicode::NarrowString  m_name;
	LocalizedString        m_srcString;
	LocalizedString        m_translatedString;

	bool                   m_hasTranslatedString;

	bool                   m_modified [2];
};
//-----------------------------------------------------------------
inline bool LocalizedStringPair::getHasTranslatedString() const
{
	return m_hasTranslatedString;
}
//-----------------------------------------------------------------

inline bool LocalizedStringPair::isOutOfDate() const
{
	return m_hasTranslatedString && (m_translatedString.getSourceCrc() != m_srcString.getCrc());
}
//-----------------------------------------------------------------

inline bool LocalizedStringPair::isModified(int index) const
{
	return m_modified [index];
}
//-----------------------------------------------------------------

inline void LocalizedStringPair::setModified(int index, bool b)
{
	m_modified [index] = b;
}
//-----------------------------------------------------------------

inline const Unicode::NarrowString &    LocalizedStringPair::getName() const
{
	return m_name;
}
//-----------------------------------------------------------------

inline void                             LocalizedStringPair::setName(const Unicode::NarrowString & name)
{
	if(m_name != name)
	{
		m_name = name;
		m_modified [0] = m_modified [1] = true;
	}
}
//-----------------------------------------------------------------
inline LocalizedString &                LocalizedStringPair::getSourceString()
{
	m_modified [0] = true;
	return m_srcString;  //lint !e1536 // exposing member
}
//-----------------------------------------------------------------

inline LocalizedString &                LocalizedStringPair::getTranslatedString()
{
	m_modified [1] = true;
	return m_translatedString;  //lint !e1536 // exposing member
}
//-----------------------------------------------------------------
inline const LocalizedString &                LocalizedStringPair::getSourceString() const
{
	return m_srcString;
}
//-----------------------------------------------------------------

inline const LocalizedString &                LocalizedStringPair::getTranslatedString() const
{
	return m_translatedString;
}
// ======================================================================

#endif
