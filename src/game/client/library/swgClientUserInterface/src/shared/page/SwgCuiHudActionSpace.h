//======================================================================
//
// SwgCuiHudActionSpace.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiHudActionSpace_H
#define INCLUDED_SwgCuiHudActionSpace_H

#include "swgClientUserInterface/SwgCuiHudAction.h"

class SwgCuiHudWindowManagerSpace;

//======================================================================

class SwgCuiHudActionSpace : public SwgCuiHudAction
{
public:
	                         SwgCuiHudActionSpace (SwgCuiHud & mediator);
	                         ~SwgCuiHudActionSpace ();

	bool                     performAction (const std::string & id, const Unicode::String & params) const;

private:
	                         SwgCuiHudActionSpace ();
	                         SwgCuiHudActionSpace (const SwgCuiHudActionSpace & rhs);
	SwgCuiHudActionSpace & operator= (const SwgCuiHudActionSpace & rhs);

	SwgCuiHudWindowManagerSpace & getWindowManagerSpace () const;

private:

	mutable int m_toggleDebounceCounterMsBooster;
	mutable int m_toggleDebounceCounterMsTargetLock;
};

//======================================================================

#endif
