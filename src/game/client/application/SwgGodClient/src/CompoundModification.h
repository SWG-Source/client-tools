// ======================================================================
//
// CompoundModification.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CompoundModification_H
#define INCLUDED_CompoundModification_H

#include "Modification.h"

// ======================================================================

/**
* A CompoundModification represents a set of Modification objects which
* are undone or redone as a group.  CompoundModification itself implements
* the Modification interface.
*
* To use a CompoundModification, call addModification on it one or more times,
* then call end ().  After end () is called, the CompoundModification is in
* a usable state.  Do not call end () multiple times.
*/

class CompoundModification : public Modification
{
public:
	CompoundModification  ();
	~CompoundModification ();

	void addModification (Modification * mod);
	void end             ();
	bool undo            ();
	bool redo            ();

private:
	//disabled
	CompoundModification (const CompoundModification & rhs);
	CompoundModification & operator= (const CompoundModification & rhs);

private:
	typedef stdvector <Modification*>::fwd ModList;
	ModList*              m_mods;
	bool                  m_complete;
};

// ======================================================================

#endif
