// ======================================================================
//
// Modification.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Modification_H
#define INCLUDED_Modification_H

// ======================================================================

/**
* Modification is an interface that represents an object which knows how to
* Undo and Redo operations.
* @see CompoundModification
* @see ModificationHistory
*/
class Modification
{
public:
	virtual bool   undo() = 0;
	virtual bool   redo() = 0;
	virtual       ~Modification() {}

protected:
	 Modification() {}

private:
	//disabled
	Modification(const Modification & rhs);
	Modification& operator=(const Modification& rhs);
};

// ======================================================================

#endif
