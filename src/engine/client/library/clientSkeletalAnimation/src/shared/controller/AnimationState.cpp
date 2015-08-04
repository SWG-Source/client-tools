// ======================================================================
//
// AnimationState.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/AnimationState.h"

#include "clientSkeletalAnimation/AnimationAction.h"
#include "clientSkeletalAnimation/AnimationStateLink.h"
#include "clientSkeletalAnimation/AnimationStatePath.h"
#include "sharedFoundation/CrcLowerString.h"

// ======================================================================

AnimationState::AnimationState()
{
}

// ----------------------------------------------------------------------

AnimationState::~AnimationState()
{
}

// ----------------------------------------------------------------------
/**
 * Convenience function to retrieve the child animation state from the
 * given the AnimationStateNameId of the directly attached child state.
 *
 * This function is built on top of the mapping function to convert
 * child state animation name id into an index to the child.  If the
 * caller of this function will repeatedly want to call this function
 * with the same AnimationStateNameId starting from the same parent
 * state, it would be more efficient to call the mapping function and
 * use the other form of this function.
 *
 * @param childStateId  the AnimationStateNameId representing the desired
 *                      child animation state to retrieve.
 *
 * @return  the child animation state from the given the AnimationStateNameId 
 *          of the directly attached child state.
 */

const AnimationState *AnimationState::getConstChildState(const AnimationStateNameId &childStateId) const
{
	int  childStateIndex = -1;

	if (lookupChildStateIndex(childStateId, childStateIndex))
		return &(getConstChildState(childStateIndex));
	else
		return 0;
}

// ----------------------------------------------------------------------
/**
 * Convenience function to retrieve the AnimationStateLink from this
 * state to the specified path, considering only a portion of the
 * full state path.
 *
 * @param path                  the path to the state for which a link is requested.
 * @param pathLengthToConsider  only this number of states from path are considered
 *                              to be the actual path for which a link is desired.
 *
 * @return  the AnimationStateLink from this state to the specified path.  Returns
 *          NULL if this state doesn't have a link to the specified path.
 */

const AnimationStateLink *AnimationState::getConstLink(const AnimationStatePath &path, int pathLengthToConsider) const
{
	const int linkCount = getLinkCount();
	for (int i = 0; i < linkCount; ++i)
	{
		const AnimationStateLink &link = getConstLink(i);

		if (link.getDestinationPath().isEqualTo(path, pathLengthToConsider))
		{
			// Found the link.
			return &link;
		}
	}

	//-- Didn't find the link, return NULL.
	return 0;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the action data for given action name associated with this state.
 *
 * The action may not be defined for the state.  In this case, the function
 * returns NULL.  The caller may want to check the parent state if
 * inherited action behavior is desired.
 * 
 * @todo this function should be part of the virtual interface.  Actions
 *       should be stored in crc order so I can quickly locate the right
 *       action.  This is an inefficient [O(n) vs. O(log n) brute-force 
 *       search algorithm.
 *
 * @param actionName  the name of the action for which action data should be returned.
 *
 * @return  the action data for the given action name associated with this state.
 */

const AnimationAction *AnimationState::getConstActionByName(const CrcLowerString &actionName) const
{
	const int actionCount = getActionCount();
	for (int i = 0; i < actionCount; ++i)
	{
		const AnimationAction &action = getConstAction(i);
		if (action.getName() == actionName)
			return &action;
	}

	//-- Didn't find the action.
	return 0;
}

// ======================================================================
