// ======================================================================
//
// AnimationEditorGameWorld.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/AnimationEditorGameWorld.h"

#include "clientGame/Game.h"
#include "clientGame/NetworkScene.h"
#include "clientSkeletalAnimation/AnimationEnvironment.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/Object.h"

// ======================================================================

namespace AnimationEditorGameWorldNamespace
{
	AnimationEnvironment s_animationEnvironment;
}

// ======================================================================
// class AnimationEditorGameWorld: public static member functions
// ======================================================================

/**
 * Return the focus Object instance for the AnimationEditor.
 *
 * The focus Object is the player's selected target if one exists.  If
 * no target is selected, the player is the focus Object.
 *
 * @return  the focus Object instance for the AnimationEditor.
 */

Object *AnimationEditorGameWorld::getFocusObject()
{
	//-- Try to get the selected target.
	Object *focusObject = CuiCombatManager::getCombatTarget().getObject();
	if (!focusObject)
		focusObject = CuiCombatManager::getLookAtTarget().getObject();

	if (!focusObject)
	{
		// get the scene
		NetworkScene* networkScene = dynamic_cast<NetworkScene*> (Game::getScene ());
		if (!networkScene)
		{
			WARNING(true, ("Could not get the NetworkScene."));
			return 0;
		}

		// Get the player
		focusObject = networkScene->getPlayer();
	}

	WARNING(!focusObject, ("Failed to get the player or a target object from the NetworkScene."));

	//-- Return the focus Object.
	return focusObject;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the AnimationEnvironment instance for the focus Object.
 *
 * If the focus Object is NULL, or does not have a skeletal Appearance,
 * a default AnimationEnvironment instance is returned.
 *
 * @return  the AnimationEnvironment instance for the focus Object.
 */

AnimationEnvironment &AnimationEditorGameWorld::getFocusObjectAnimationEnvironment()
{
	// Get the object.
	Object *const focusObject = getFocusObject();
	if (!focusObject)
		return AnimationEditorGameWorldNamespace::s_animationEnvironment;

	// Get the appearance for the object.
	SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(focusObject->getAppearance());
	if (!appearance)
		return AnimationEditorGameWorldNamespace::s_animationEnvironment;

	return appearance->getAnimationEnvironment();
}

// ======================================================================
