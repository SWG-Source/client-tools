//=============================================================================
//
// SwgCuiStatusFactory.cpp
// copyright(c) 2004 Sony Online Entertainment
//
//=============================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiStatusFactory.h"

#include "swgClientUserInterface/SwgCuiStatusGround.h"
#include "swgClientUserInterface/SwgCuiStatusSpace.h"

//-----------------------------------------------------------------------------

SwgCuiMfdStatus * SwgCuiStatusFactory::createStatusPage(Game::SceneType sceneType, UIPage & page, bool const isGroupType)
{
	SwgCuiMfdStatus * statusObject = NULL;

	switch(sceneType) 
	{
		case Game::ST_space:
			statusObject = new SwgCuiStatusSpace(page);
			break;

		case Game::ST_ground: //fallthrough
		default:
			statusObject = new SwgCuiStatusGround(page, isGroupType ? SwgCuiStatusGround::ST_group : SwgCuiStatusGround::ST_target);
			break;
	}

	return statusObject;
}
