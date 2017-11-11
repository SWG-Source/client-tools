// ======================================================================
//
// LocalizedStringPair.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "StdAfx.h"
#include "LocalizedStringPair.h"

#include "LocalizedStringTableReaderWriter.h"

// ======================================================================

LocalizedStringPair::LocalizedStringPair () :
m_name (),
m_srcString (),
m_translatedString (),
m_hasTranslatedString (false)
{
	m_modified [0] = false;
	m_modified [1] = false;
}

//-----------------------------------------------------------------

LocalizedStringPair::LocalizedStringPair (const Unicode::NarrowString & name, const LocalizedString & srcString, const LocalizedString * translatedString) :
m_name (name),
m_srcString (srcString),
m_translatedString (),
m_hasTranslatedString (translatedString != 0)
{
	if (translatedString)
	{
		m_translatedString = *translatedString;
		m_translatedString.setSourceCrc(m_srcString.getCrc());
	}

	m_modified [0] = false;
	m_modified [1] = false;
}

//-----------------------------------------------------------------

LocalizedStringPair::LocalizedStringPair (const LocalizedStringPair & rhs) :
m_name (rhs.m_name),
m_srcString (rhs.m_srcString),
m_translatedString (rhs.m_translatedString),
m_hasTranslatedString (rhs.m_hasTranslatedString)
{
	m_modified [0] = false;
	m_modified [1] = false;
}

//-----------------------------------------------------------------

LocalizedStringPair &            LocalizedStringPair::operator=    (const LocalizedStringPair & rhs)
{
	if (this != &rhs)
	{
		//-- setName handles setting the modified flags
		setName (rhs.m_name);
		
		if (!(m_srcString == rhs.m_srcString))
		{
			m_srcString    = rhs.m_srcString;
			m_modified [0] = true;
		}
		
		if (!(m_translatedString == rhs.m_translatedString))
		{
			m_translatedString = rhs.m_translatedString;
			m_modified [1] = true;
		}
		
		if (m_hasTranslatedString != rhs.m_hasTranslatedString)
		{
			m_hasTranslatedString = rhs.m_hasTranslatedString;
			m_modified [1] = true;
		}
	}

	return *this;
}

//-----------------------------------------------------------------

LocalizedString::id_type         LocalizedStringPair::getId () const
{
	return m_srcString.getId ();
}

//-----------------------------------------------------------------

void LocalizedStringPair::setHasTranslatedString (bool b)
{
	if (m_hasTranslatedString == b)
		return;

	m_modified [1] = true;

	m_hasTranslatedString = b;

	if (b)
	{
		LocalizedStringTableRW::str_setString(m_translatedString, Unicode::String ());
		LocalizedStringTableRW::str_getId(m_translatedString) = m_srcString.getId ();
		LocalizedStringTableRW::str_getSourceCrc(m_translatedString) = m_srcString.getCrc();

		m_translatedString.buildCrc();
	}
}


//-----------------------------------------------------------------

void LocalizedStringPair::updateCrc()
{
	LocalizedStringTableRW::str_getSourceCrc(m_translatedString) = m_srcString.getCrc();
}

// ======================================================================
