// ======================================================================
//
// Conversation.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgContentBuilder.h"
#include "Conversation.h"

// ======================================================================

Conversation::Conversation () :
	m_stringMap ()
{
}

// ----------------------------------------------------------------------

Conversation::~Conversation ()
{
}

// ----------------------------------------------------------------------

void Conversation::clear ()
{
	m_stringMap.clear ();
}

// ----------------------------------------------------------------------

std::string const & Conversation::getString (std::string const & key) const
{
	return const_cast<Conversation&> (*this).m_stringMap [key];
}

// ----------------------------------------------------------------------

void Conversation::setString (std::string const & key, std::string const & value)
{
	m_stringMap [key] = value;
}

// ----------------------------------------------------------------------

void Conversation::getStringList (Conversation::StringList & stringList)
{
	stringList.clear ();

	StringMap::const_iterator end = m_stringMap.end ();
	for (StringMap::const_iterator iter = m_stringMap.begin (); iter != end; ++iter)
		stringList.push_back (std::make_pair (iter->first, iter->second));
}

// ======================================================================
