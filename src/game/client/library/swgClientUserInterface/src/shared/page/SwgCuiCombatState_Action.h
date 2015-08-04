//======================================================================
//
// SwgCuiCombatState_Action.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiCombatState_Action_H
#define INCLUDED_SwgCuiCombatState_Action_H

//======================================================================

#include "swgClientUserInterface/SwgCuiCombatState.h"
#include "clientUserInterface/CuiAction.h"

//----------------------------------------------------------------------

class SwgCuiCombatState::Action : public CuiAction
{
public:
	
	explicit  Action (SwgCuiCombatState & mediator);
	         ~Action ();
	bool      performAction (const std::string & id, const Unicode::String &) const;
	
private:
	Action ();
	Action (const Action & rhs);
	Action & operator= (const Action & rhs);

	SwgCuiCombatState & m_mediator;
};

//======================================================================

#endif
