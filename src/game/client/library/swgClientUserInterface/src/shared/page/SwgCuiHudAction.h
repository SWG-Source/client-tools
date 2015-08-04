//======================================================================
//
// SwgCuiHudAction.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiHudAction_H
#define INCLUDED_SwgCuiHudAction_H

//======================================================================

#include "swgClientUserInterface/SwgCuiHud.h"
#include "clientUserInterface/CuiAction.h"

//----------------------------------------------------------------------

class SwgCuiHudAction : public CuiAction
{
public:
	
	explicit          SwgCuiHudAction (SwgCuiHud & mediator);
	virtual          ~SwgCuiHudAction () = 0;
	virtual bool      performAction (const std::string & id, const Unicode::String &) const;
	
protected:
	SwgCuiHud &               getMediator () const;
	SwgCuiHudWindowManager &  getWindowManager () const;
	
private:
	SwgCuiHudAction ();
	SwgCuiHudAction (const SwgCuiHudAction & rhs);
	SwgCuiHudAction & operator= (const SwgCuiHudAction & rhs);

	SwgCuiHud &    m_mediator;
	mutable bool   m_nextToggleUpDeactivesPointer;
	mutable int    m_toggleDebounceCounter;
	mutable bool   m_nextToggleUpDeactivesNames;
	mutable float  m_toggleDownTimeNames;

};

//----------------------------------------------------------------------

inline SwgCuiHud & SwgCuiHudAction::getMediator () const
{
	return m_mediator;
}

//======================================================================

#endif
