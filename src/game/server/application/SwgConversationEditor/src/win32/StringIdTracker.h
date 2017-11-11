// ======================================================================
//
// StringIdTracker.h
//
// copyright 2005, sony online entertainment
// 
// ======================================================================

#ifndef INCLUDED_StringIdTracker_H
#define INCLUDED_StringIdTracker_H

// ======================================================================

#include <set>
#include <string>

// ======================================================================

class StringIdTracker
{
public:

	StringIdTracker();
	~StringIdTracker();

	std::string getUniqueStringId();
	bool isUniqueStringId(std::string const & stringId);
	void insertUniqueStringId(std::string const & stringId);

private:

	typedef std::set<std::string> StringIdSet;

	StringIdSet m_stringIdSet;
	uint32 m_nextStringId;
};

// ======================================================================

#endif
