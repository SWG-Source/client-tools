// ======================================================================
//
// AnimationStateHierarchyTemplate.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/AnimationStateHierarchyTemplate.h"

#include "clientSkeletalAnimation/AnimationState.h"
#include "clientSkeletalAnimation/AnimationStateHierarchyTemplateList.h"
#include "clientSkeletalAnimation/AnimationStatePath.h"
#include "sharedFoundation/CrcLowerString.h"

#include <algorithm>

// ======================================================================
// class AnimationStateHierarchyTemplate: public member functions
// ======================================================================

void AnimationStateHierarchyTemplate::release() const
{
	//-- Decrement the reference count.
	--m_referenceCount;

	//-- Check if the instance should be deleted.
	if (m_referenceCount < 1)
	{
		//-- Delete the instance.
		DEBUG_WARNING(m_referenceCount < 0, ("bad reference count handling: %d (%s)", m_referenceCount, getName().getString()));

		AnimationStateHierarchyTemplateList::stopTracking(*this);
		delete const_cast<AnimationStateHierarchyTemplate*>(this);
	}
}

// ----------------------------------------------------------------------
/**
 * Return the const AnimationState identified by the given state path.
 *
 * @param path  the path to the state that will be retrieved.
 *
 * @return  the const AnimationState identified by the given state path.
 *          If the state does not exist in the hierarchy, this will return
 *          NULL.
 */

const AnimationState *AnimationStateHierarchyTemplate::getConstAnimationState(const AnimationStatePath &path) const
{
	//-- Get the root state.
	const AnimationState *state      = getConstRootAnimationState();
	const int             pathLength = path.getPathLength();

	//-- Traverse the path (skip the root).
	for (int i = 1; (i < pathLength) && (state != NULL); ++i)
	{
		state = state->getConstChildState(path.getState(i));
	}

	return state;
}

// ----------------------------------------------------------------------

const AnimationState *AnimationStateHierarchyTemplate::getConstAnimationState(const AnimationStatePath &path, int partialPathLength) const
{
	//-- Get the root state.
	const AnimationState *state      = getConstRootAnimationState();
	const int             pathLength = std::min(path.getPathLength(), partialPathLength);

	//-- Traverse the path (skip the root).
	for (int i = 1; (i < pathLength) && (state != NULL); ++i)
	{
		state = state->getConstChildState(path.getState(i));
	}

	return state;
}


// ======================================================================
// class AnimationStateHierarchyTemplate: protected member functions
// ======================================================================

AnimationStateHierarchyTemplate::AnimationStateHierarchyTemplate(const CrcString &name) :
	m_name(name.getString(), name.getCrc()),
	m_referenceCount(0)
{
}


// ----------------------------------------------------------------------

AnimationStateHierarchyTemplate::~AnimationStateHierarchyTemplate()
{
}

// ======================================================================


