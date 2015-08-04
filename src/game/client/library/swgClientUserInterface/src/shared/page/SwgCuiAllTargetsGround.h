//======================================================================
//
// SwgCuiAllTargetsGround.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiAllTargetsGround_H
#define INCLUDED_SwgCuiAllTargetsGround_H

//======================================================================

#include "swgClientUserInterface/SwgCuiAllTargets.h"

//----------------------------------------------------------------------

class SwgCuiAllTargetsGround :
public SwgCuiAllTargets
{
public:
	SwgCuiAllTargetsGround(UIPage & page);

private:
	virtual bool isWaypointVisible (Vector & effectiveWaypointPosition_o, Camera const & camera, Appearance const * appearance) const;

private:
	SwgCuiAllTargetsGround();
	SwgCuiAllTargetsGround(SwgCuiAllTargetsGround const &);
	SwgCuiAllTargetsGround &operator=(SwgCuiAllTargetsGround const &);
};

//======================================================================

#endif
