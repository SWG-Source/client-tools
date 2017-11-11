//======================================================================
//
// SwgCuiAllTargetsGround.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAllTargetsGround.h"

#include "clientObject/GameCamera.h"
#include "sharedObject/Appearance.h"

//======================================================================

SwgCuiAllTargetsGround::SwgCuiAllTargetsGround(UIPage & page) :
SwgCuiAllTargets("SwgCuiAllTargetsGround", page)
{
}

//----------------------------------------------------------------------

bool SwgCuiAllTargetsGround::isWaypointVisible(Vector & effectiveWaypointPosition_o, Camera const & camera, Appearance const * appearance) const
{
	if (!appearance || !appearance->getRenderedThisFrame())
		return false;
	
	const Volume & volume = camera.getFrustumVolume ();
	return volume.contains (effectiveWaypointPosition_o);	
}

//======================================================================
