//=============================================================================
//
// SwgCuiStatusFactory.h
// copyright(c) 2004 Sony Online Entertainment
//
//=============================================================================

#ifndef INCLUDED_SwgCuiStatusFactory_H
#define INCLUDED_SwgCuiStatusFactory_H

//=============================================================================

#include "swgClientUserInterface/SwgCuiMfdStatus.h"
#include "clientGame/Game.h"

//-----------------------------------------------------------------------------

class SwgCuiStatusFactory
{
public:
	static SwgCuiMfdStatus * createStatusPage(Game::SceneType sceneType, UIPage & page, bool const isGroupType = false);
};

//=============================================================================

#endif
