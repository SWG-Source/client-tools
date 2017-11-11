// ======================================================================
//
// TrackAnimationController_PhysicalTrackTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/TrackAnimationController_PhysicalTrackTemplate.h"

#include "clientSkeletalAnimation/TrackAnimationController_AnimationFactory.h"
#include "sharedFoundation/PointerDeleter.h"

#include <algorithm>

// ======================================================================
// class TrackAnimationController::PhysicalTrackTemplate
// ======================================================================

TrackAnimationController::PhysicalTrackTemplate::PhysicalTrackTemplate(const CrcLowerString &trackName, const AnimationTrackId &trackId, int physicalTrackIndex, AnimationFactory *animationFactory, bool allowSameTrackTrumping) :
	TrackTemplate(trackName, trackId),
	m_physicalTrackIndex(physicalTrackIndex),
	m_postEnterIdleActions(),
	m_preExitIdleActions(),
	m_animationFactory(animationFactory),
	m_allowSameTrackTrumping(allowSameTrackTrumping)
{
}

// ----------------------------------------------------------------------

TrackAnimationController::PhysicalTrackTemplate::~PhysicalTrackTemplate()
{
	delete m_animationFactory;

	std::for_each(m_postEnterIdleActions.begin(), m_postEnterIdleActions.end(), PointerDeleter());
	std::for_each(m_preExitIdleActions.begin(), m_preExitIdleActions.end(), PointerDeleter());
}

// ----------------------------------------------------------------------

SkeletalAnimation *TrackAnimationController::PhysicalTrackTemplate::fetchInitialAnimation(const TransformNameMap &transformNameMap, TrackAnimationController &animationController) const
{
	if (m_animationFactory)
		return m_animationFactory->fetchSkeletalAnimation(transformNameMap, animationController);
	else
		return 0;
}

// ----------------------------------------------------------------------

bool TrackAnimationController::PhysicalTrackTemplate::hasTrackIndexResolver() const
{
	// physical tracks do not have track index resolvers: they always refer
	// to the same physical track.
	return false;
}

// ----------------------------------------------------------------------

int TrackAnimationController::PhysicalTrackTemplate::getTrackIndexResolverIndex() const
{
	// this is invalid to call when hasTrackIndexResolver() returns false.
	DEBUG_FATAL(true, ("caller called getTrackIndexResolverIndex() when hasTrackIndexResolver() returned false"));
	return 0;
}

// ----------------------------------------------------------------------

int TrackAnimationController::PhysicalTrackTemplate::getPhysicalTrackIndex() const
{
	return m_physicalTrackIndex;
};

// ======================================================================
