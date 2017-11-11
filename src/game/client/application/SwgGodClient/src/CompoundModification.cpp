// ======================================================================
//
// CompoundModification.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "CompoundModification.h"
#include "sharedFoundation/PointerDeleter.h"

#include <algorithm>

// ======================================================================

/**
 * Create an empty CompoundModification
 */
CompoundModification::CompoundModification()
: Modification(),
  m_mods(0),
  m_complete(false)
{
	m_mods = new ModList;
}

//-----------------------------------------------------------------

CompoundModification::~CompoundModification()
{
	std::for_each(m_mods->begin(), m_mods->end(), PointerDeleter());
	delete m_mods;
	m_mods = 0;
}

//-----------------------------------------------------------------

/**
 * Add one Modification.
 */
void CompoundModification::addModification(Modification* mod)
{
	DEBUG_FATAL(m_complete,("Add called on ended compound.\n"));

	m_mods->push_back(mod);
}
//-----------------------------------------------------------------

/**
 * Stop adding Modifications to this CompoundModification and prepare for use
 */
void CompoundModification::end()
{
	DEBUG_FATAL(m_complete,("End called multiple times on compound.\n"));
	m_complete = true;
}

//-----------------------------------------------------------------

/**
 * Execute the undo's on the members of the CompoundModification, in the
 * reverse order in which they were added
 */
bool CompoundModification::undo()
{
	DEBUG_FATAL(!m_complete,("undo before compound end.\n"));

	for(ModList::reverse_iterator rit = m_mods->rbegin(); rit != m_mods->rend(); ++rit) //lint !e55 !e81 // stl bug
	{
		Modification* const mod = NON_NULL(*rit);
		if(!mod->undo())
			return false;
	}
	return true;	
}
//-----------------------------------------------------------------

/**
 * Execute the redo's on the members of the CompoundModification, in the
 * same order in which they were added
 */
bool CompoundModification::redo()
{
	DEBUG_FATAL(!m_complete,("redo before compound end.\n"));

	for(ModList::iterator it = m_mods->begin(); it != m_mods->end(); ++it)
	{
		Modification* const mod = NON_NULL(*it);
		if(!mod->redo())
			return false;
	}
	return true;
}

// ======================================================================
