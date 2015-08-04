//======================================================================
//
// SwgCuiCombatState_Action.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCombatState_Action.h"

#include "sharedFoundation/GameControllerMessage.h"
#include "clientUserInterface/CuiActionManager.h"

//======================================================================

SwgCuiCombatState::Action::Action (SwgCuiCombatState & mediator) :
CuiAction (),
m_mediator (mediator)
{

}

//----------------------------------------------------------------------

SwgCuiCombatState::Action::~Action ()
{
	CuiActionManager::removeAction (this);
}

//----------------------------------------------------------------------

bool  SwgCuiCombatState::Action::performAction (const std::string & id, const Unicode::String &) const
{
	UNREF (id);
	return false;
}

//======================================================================
