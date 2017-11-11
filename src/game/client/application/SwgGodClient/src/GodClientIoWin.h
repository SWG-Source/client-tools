// ======================================================================
//
// GodClientIoWin.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GodClientIoWin_H
#define INCLUDED_GodClientIoWin_H

#include "sharedIoWin/IoWin.h"

// ======================================================================
/**
* The GodClient draws its special effects (selection indicators, etc) through
* a dedicated IoWin which is kept above the GroundScene.  The GodClientIoWin
* currently does no event handling.
*/
class GodClientIoWin : public IoWin
{
public:
	void draw() const;
	GodClientIoWin();

private:
	//disabled
	GodClientIoWin(const GodClientIoWin& rhs);
	GodClientIoWin& operator=(const GodClientIoWin& rhs);
};

// ======================================================================

#endif
