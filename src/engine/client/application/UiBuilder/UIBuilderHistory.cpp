// ======================================================================
//
// UIBuilderHistory.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstUiBuilder.h"
#include "UIBuilderHistory.h"

#include <vector>

//-----------------------------------------------------------------
namespace
{
	typedef std::vector<std::string> HistoryVector;
	HistoryVector * ms_history;
	size_t          ms_index;
	bool            ms_installed;
}
//-----------------------------------------------------------------

void UIBuilderHistory::install ()
{
	assert (!ms_installed);
	ms_history = new HistoryVector;
	ms_index = 0;
	ms_installed = true;
}

//-----------------------------------------------------------------

void UIBuilderHistory::remove ()
{
	assert (ms_installed);
	delete ms_history;
	ms_history = 0;
	ms_installed = false;
}

//-----------------------------------------------------------------

bool UIBuilderHistory::back (std::string & path )
{
	if (!backValid ())
		return false;

	path = (*ms_history) [--ms_index];

	return true;
}

//-----------------------------------------------------------------

bool UIBuilderHistory::forward (std::string & path)
{
	if (!forwardValid ())
		return false;

	path = (*ms_history) [++ms_index];
	return true;
}

//-----------------------------------------------------------------

void UIBuilderHistory::pushNode (const std::string & path)
{
	assert (ms_installed);
	if (!ms_history->empty () && ms_index < ms_history->size () - 1)
	{
		HistoryVector::iterator it = ms_history->begin ();
		std::advance (it, ms_index + 1);
		ms_history->erase (it, ms_history->end ());
	}

	if (!ms_history->empty ())
		++ms_index;

	ms_history->push_back (path);
}

//-----------------------------------------------------------------

bool UIBuilderHistory::backValid ()
{
	assert (ms_installed);
	return (ms_index > 0);
}

//-----------------------------------------------------------------

bool UIBuilderHistory::forwardValid ()
{
	assert (ms_installed);
	return (ms_index < ms_history->size () - 1);
}

//----------------------------------------------------------------------

bool UIBuilderHistory::isInstalled()
{
	return ms_installed;
}

// ======================================================================
