// ======================================================================
//
// ModificationHistory.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ModificationHistory_H
#define INCLUDED_ModificationHistory_H

// ======================================================================
#include "Singleton/Singleton.h"

class Modification;
class CompoundModification;

//-----------------------------------------------------------------

/**
* A ModificationHistory represents an undo-redo history.
*/
class ModificationHistory : public Singleton<ModificationHistory>
{
public:

	ModificationHistory  ();
	~ModificationHistory ();
	void    startCompoundModification ();
	void    endCompoundModification   ();
	void    addModification (Modification* mod);
	bool    undo    ();
	bool    redo    ();
	bool    canUndo () const;
	bool    canRedo () const;

private:
	//disabled
	ModificationHistory(const ModificationHistory& rhs);
	ModificationHistory& operator=(const ModificationHistory& rhs);

private:
	typedef std::vector<Modification *> NodeList;
	NodeList                  m_nodes;
	size_t                    m_index;
	CompoundModification*     m_compound;
};
//-----------------------------------------------------------------

/**
* canUndo returns true if there exists a Modification before the current m_index.
* No undo/redo can be performed while a CompoundModification construction is
* in progress.
*/
inline bool ModificationHistory::canUndo() const
{
	return !m_compound && !m_nodes.empty() && m_index > 0;
}
//-----------------------------------------------------------------
/**
* canUndo returns true if there exists a Modification at the current m_index.
* No undo/redo can be performed while a CompoundModification construction is
* in progress.
*/

inline bool ModificationHistory::canRedo() const 
{
	return !m_compound && !m_nodes.empty() && m_index < m_nodes.size();
}
// ======================================================================

#endif
