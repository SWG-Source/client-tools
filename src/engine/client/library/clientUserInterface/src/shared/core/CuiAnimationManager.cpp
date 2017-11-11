//======================================================================
//
// CuiAnimationManager.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiAnimationManager.h"

#include "clientGame/ClientObject.h"
#include "clientGame/CreatureController.h"
#include "clientGame/CreatureObject.h"
#include "clientSkeletalAnimation/SkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/StateHierarchyAnimationController.h"
#include "clientSkeletalAnimation/TransformAnimationResolver.h"
#include "sharedFoundation/CrcLowerString.h"

//@todo: these include paths are errors
#include "swgSharedUtility/Postures.h"
#include "swgSharedUtility/States.h"

//======================================================================

/**
 * Play an emote.
 *
 * Note: this will not play an action.  It will only play an emote.
 */

bool CuiAnimationManager::attemptPlayEmote (ClientObject & source, ClientObject * target, const std::string & emoteName)
{
	if (emoteName.empty () || emoteName [0] == '~')
		return false;

	int  animationId    = 0;
	bool animationIsAdd = false;

	CreatureObject * const creatureObject = source.asCreatureObject ();
	if (creatureObject)
	{
		//-- only allow actions if we're standing (upright)
		// @todo this line will prevent facial-only emotes from working when sitting or any-other non-standing posture.
		//       I don't think we want this for non-body animations.
		if ((creatureObject->getVisualPosture () != Postures::Upright || creatureObject->getState (States::Swimming)) && _strnicmp (emoteName.c_str (), "face_", 5))
			return false;
		
		//-- Play animation with face tracking.
		if (target && (&source != target))
		{
			CreatureController *const controller = safe_cast<CreatureController*> (creatureObject->getController ());
			if (!controller)
			{
				WARNING (true, ("CuiAnimationManager::attemptPlayEmote source object [%s] has no CreatureController", Unicode::wideToNarrow (source.getLocalizedName ()).c_str ()));
			}
			else
			{
				//-- If current speed is zero, face the target before playing.  This will handle cases where
				//   face tracking doesn't work, such as when playing facial-only animations.
				if (controller->getCurrentSpeed () <= 0.0f)
					controller->face (target->getPosition_w ());
				
				controller->playAnimationWithFaceTracking (CrcLowerString (emoteName.c_str ()), *target, animationId, animationIsAdd, NULL);				
				return true;
			}
		}
	}

	SkeletalAppearance2 * const skelApp = source.getAppearance ()->asSkeletalAppearance2 ();

	if (skelApp)
	{
		skelApp->getAnimationResolver ().playAction (CrcLowerString (emoteName.c_str ()), animationId, animationIsAdd, NULL);
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

/**
*
*/

bool CuiAnimationManager::isValidEmote (const ClientObject & , const ClientObject * , const Unicode::NarrowString & emoteName)
{
	if (emoteName.empty () || emoteName [0] == '~')
		return false;

	//@todo: implement
	return true;
}

//======================================================================
