// ======================================================================
//
// TrackAnimationController_TrackTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/TrackAnimationController_TrackTemplate.h"

// ======================================================================
// class TrackAnimationController::TrackTemplate
// ======================================================================

TrackAnimationController::TrackTemplate::~TrackTemplate()
{
}

// ======================================================================

TrackAnimationController::TrackTemplate::TrackTemplate(const CrcLowerString &trackName, const AnimationTrackId &trackId)
:	m_trackName(trackName),
	m_trackId(trackId)
{
}

// ======================================================================
