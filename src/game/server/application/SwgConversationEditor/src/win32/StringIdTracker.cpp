// ======================================================================
//
// StringIdTracker.cpp
//
// copyright 2005, sony online entertainment
// 
// ======================================================================

#include "FirstSwgConversationEditor.h"
#include "StringIdTracker.h"

#include "sharedFoundation/FormattedString.h"
#include "sharedFoundationTypes/FoundationTypes.h"

// ======================================================================

StringIdTracker::StringIdTracker () :
	m_stringIdSet (),
	m_nextStringId (0)
{
}

// ----------------------------------------------------------------------

StringIdTracker::~StringIdTracker ()
{
}

// ----------------------------------------------------------------------

std::string StringIdTracker::getUniqueStringId()
{
	for (;;)
	{
		std::string const stringId = FormattedString<64>().sprintf("%d", ++m_nextStringId);
		StringIdSet::iterator iter = m_stringIdSet.find(stringId);
		if (iter == m_stringIdSet.end())
		{
			m_stringIdSet.insert(stringId);
			return stringId;
		}

		if (m_nextStringId == 0)
			break;
	}

	FATAL(true, ("Could not find unique stringId after %d tries", static_cast<uint32>(-1)));
	return std::string("error");
}

// ----------------------------------------------------------------------

bool StringIdTracker::isUniqueStringId(std::string const & stringId)
{
	return m_stringIdSet.find(stringId) == m_stringIdSet.end();
}

// ----------------------------------------------------------------------

void StringIdTracker::insertUniqueStringId(std::string const & stringId)
{
	//-- Insert the string id into the list of unique ids
	m_stringIdSet.insert(stringId);
}

// ======================================================================
