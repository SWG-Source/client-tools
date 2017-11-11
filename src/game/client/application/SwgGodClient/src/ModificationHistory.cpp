// ======================================================================
//
// ModificationHistory.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "ModificationHistory.h"
#include "Modification.h"
#include "CompoundModification.h"

// ======================================================================

/**
* Constructs an empty ModificationHistory
*
*/

ModificationHistory::ModificationHistory()
: Singleton <ModificationHistory>(),
  m_nodes(),
  m_index(0),
  m_compound(0)
{
}

//-----------------------------------------------------------------

/**
* Delete all modifications in the same order that they were added.
*
*/

ModificationHistory::~ModificationHistory()
{
	for(NodeList::iterator it = m_nodes.begin(); it != m_nodes.end(); ++it)
	{
		delete *it;
		*it = 0;
	}

	m_nodes.clear();

	delete m_compound;
	m_compound = 0;
}
//-----------------------------------------------------------------

/**
* A convenience method for adding compound modifications to the ModificationHistory
* You may also simply add CompoundModifications directly.
*
*/

void ModificationHistory::startCompoundModification()
{
	DEBUG_FATAL(m_compound,("Compound modification already in progress.\n"));

	m_compound = new CompoundModification;
}
//-----------------------------------------------------------------

/**
* A convenience method for adding compound modifications to the ModificationHistory
* You may also simply add CompoundModifications directly.
*
*/

void ModificationHistory::endCompoundModification  ()
{
	NOT_NULL(m_compound);

	CompoundModification * cm = m_compound;
	m_compound = 0;
	cm->end();
	addModification(cm);
}
//-----------------------------------------------------------------

/**
* Add a modification to the History at the location indicated by m_index.
* If m_index is less than the size of m_mods, all modifications past the
* newly added modification are deleted in the same order that they were added.
*
*/

void ModificationHistory::addModification(Modification * mod)
{
	if(m_compound)
	{
		m_compound->addModification(mod);
		return;
	}

	if(m_index < m_nodes.size())
	{
		NodeList::iterator it = m_nodes.begin();
		std::advance(it, m_index);

		for(; it != m_nodes.end();)
		{
			delete *it;
			*it = 0;
			it = m_nodes.erase(it);
		}
	}

	m_nodes.push_back(mod);
	++m_index;
}
//-----------------------------------------------------------------

/**
* Undo the modification before m_index, and decrement m_index.
*
*/

bool ModificationHistory::undo()
{
	if(!canUndo())
		return false;

	NodeList::iterator it = m_nodes.begin();
	std::advance(it, --m_index);
	Modification * const mod = NON_NULL(*it);

	return mod->undo();
}

//-----------------------------------------------------------------

/**
* Redo the modification at m_index, and increment m_index
*
*/

bool ModificationHistory::redo()
{
	if(!canRedo())
		return false;

	NodeList::iterator it = m_nodes.begin();
	std::advance(it, m_index++);
	Modification * const mod = NON_NULL(*it);

	return mod->redo();
}

// ======================================================================
